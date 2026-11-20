// FILE: callbacks-firmware-level2.c

// This file holds callbacks for the VSCP protocol

/* ******************************************************************************
 * 	VSCP (Very Simple Control Protocol)
 * 	https://www.vscp.org
 *
 * The MIT License (MIT)
 *
 * Copyright (C) 2000-2026 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *	This file is part of VSCP - Very Simple Control Protocol
 *	https://www.vscp.org
 *
 * ******************************************************************************
 */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tim.h"
#include "usart.h"

#include "vscp-compiler.h"
#include "vscp-projdefs.h"

#include <flash_storage.h>
#include "watchdog.h"

#include <vscp-fifo.h>

#include "vscp-link-protocol.h"
#include "vscp-firmware-helper.h"
#include "vscp-firmware-level2.h"

#include <wiznet-ip20.h>
#include "main.h"
#include "blinky.h"
#include "vscp.h"
#include <vscp-class.h>
#include <vscp-type.h>

extern register_union_t g_registers;             // Global register storage
extern volatile uint32_t g_user_reg_millisecond; // Millisecond counter register, updated every 1 ms
extern volatile uint8_t g_user_reg_status;       // Status register, non-persistent

static int s_led_on          = 0; // LED is lit
static int s_blinking        = 0; // LED is blinking (actions 5/6, control bit 7, interval)
static uint32_t s_last_blink = 0; // Time of the last blink toggle
static int s_save_pending    = 0; // Registers changed and not yet saved
static uint32_t s_save_time  = 0; // Time of the last change

// ****************************************************************************
//                              Blinky functions
// ****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// blinky_request_save
//

void
blinky_request_save(void)
{
  s_save_pending = 1;
  s_save_time    = HAL_GetTick();
}

///////////////////////////////////////////////////////////////////////////////
// blinky_send_event
//

int
blinky_send_event(vscp_frmw2_firmware_context_t *pfw,
                  uint16_t vscp_class,
                  uint16_t vscp_type,
                  const uint8_t *pdata,
                  uint8_t size)
{
  vscp_link_ctx_t *plink = (vscp_link_ctx_t *) pfw->puserdata;

  // Only queue events for a connected client (a later client should not get old events)
  if ((NULL == plink) || (VSCP_STATE_CONNECTED != plink->sock)) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  vscp_event_t *pev = vscp_fwhlp_newEvent();
  if (NULL == pev) {
    return VSCP_ERROR_MEMORY;
  }

  if (VSCP_ERROR_SUCCESS != vscp_frmw2_setup_event(pfw, pev, size)) {
    vscp_fwhlp_deleteEvent(&pev);
    return VSCP_ERROR_MEMORY;
  }

  pev->vscp_class = vscp_class;
  pev->vscp_type  = vscp_type;
  if (size && (NULL != pdata)) {
    memcpy(pev->pdata, pdata, size);
  }

  if (!vscp_fifo_write(&plink->fifoEventsOut, pev)) {
    plink->statistics.cntOverruns++;
    vscp_fwhlp_deleteEvent(&pev);
    return VSCP_ERROR_FIFO_FULL;
  }

  plink->statistics.cntTransmitFrames++;
  plink->statistics.cntTransmitData += size;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// blinky_set_led
//

void
blinky_set_led(vscp_frmw2_firmware_context_t *pfw, int on, int report)
{
  int changed = (on ? 1 : 0) != s_led_on;
  s_led_on    = on ? 1 : 0;

  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, s_led_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
  // The button interrupt changes bit 0 of the same register
  __disable_irq();
  if (s_led_on) {
    g_user_reg_status |= BLINKY_STATUS_LED_ON;
  }
  else {
    g_user_reg_status &= ~BLINKY_STATUS_LED_ON;
  }
  __enable_irq();

  // CLASS1.INFORMATION ON/OFF for changes made from outside (not for each blink)
  if (report && changed) {
    uint8_t data[3] = { 0, g_registers.data.zone, g_registers.data.subzone }; // index, zone, subzone
    blinky_send_event(pfw,
                      VSCP_CLASS1_INFORMATION,
                      s_led_on ? VSCP_TYPE_INFORMATION_ON : VSCP_TYPE_INFORMATION_OFF,
                      data,
                      sizeof(data));
  }
}

///////////////////////////////////////////////////////////////////////////////
// blinky_init
//

void
blinky_init(vscp_frmw2_firmware_context_t *pfw)
{
  blinky_set_led(pfw, 0, 0);
  // Blink from start when the LED is enabled and an interval is set
  s_blinking   = (g_registers.data.control & BLINKY_CTRL_ENABLE_LED) && (0 != g_registers.data.blink_interval);
  s_last_blink = HAL_GetTick();
}

///////////////////////////////////////////////////////////////////////////////
// blinky_work
//

void
blinky_work(vscp_frmw2_firmware_context_t *pfw)
{
  // * * * Button * * *
  // The interrupt handler only records presses/releases; the events are made here
  __disable_irq();
  uint8_t button = g_button_events;
  g_button_events = 0;
  __enable_irq();

  if (button && (g_registers.data.control & BLINKY_CTRL_ENABLE_BTN)) {
    if (button & BLINKY_BUTTON_PRESSED) {
      if (g_registers.data.control & BLINKY_CTRL_ENABLE_BTN_TURNON) {
        // https://grodansparadis.github.io/vscp-doc-spec/#/./class1.control?id=type5
        uint8_t data[3] = { g_registers.data.button_zero_opt_byte,
                            g_registers.data.button_zone,
                            g_registers.data.button_subzone };
        blinky_send_event(pfw, VSCP_CLASS1_CONTROL, VSCP_TYPE_CONTROL_TURNON, data, sizeof(data));
      }
      if (g_registers.data.control & BLINKY_CTRL_ENABLE_BTN_START) {
        // https://grodansparadis.github.io/vscp-doc-spec/#/./class1.information?id=type25
        uint8_t data[3] = { 0, g_registers.data.button_zone, g_registers.data.button_subzone };
        blinky_send_event(pfw, VSCP_CLASS1_INFORMATION, VSCP_TYPE_INFORMATION_START, data, sizeof(data));
      }
    }
    if (button & BLINKY_BUTTON_RELEASED) {
      if (g_registers.data.control & BLINKY_CTRL_ENABLE_BTN_TURNON) {
        // TURN-OFF on release goes with TURN-ON on press (bit 3)
        // https://grodansparadis.github.io/vscp-doc-spec/#/./class1.control?id=type6
        uint8_t data[3] = { g_registers.data.button_zero_opt_byte,
                            g_registers.data.button_zone,
                            g_registers.data.button_subzone };
        blinky_send_event(pfw, VSCP_CLASS1_CONTROL, VSCP_TYPE_CONTROL_TURNOFF, data, sizeof(data));
      }
      if (g_registers.data.control & BLINKY_CTRL_ENABLE_BTN_STOP) {
        // https://grodansparadis.github.io/vscp-doc-spec/#/./class1.information?id=type24
        uint8_t data[3] = { 0, g_registers.data.button_zone, g_registers.data.button_subzone };
        blinky_send_event(pfw, VSCP_CLASS1_INFORMATION, VSCP_TYPE_INFORMATION_STOP, data, sizeof(data));
      }
    }
  }

  // * * * Blink * * *
  if (s_blinking && (g_registers.data.control & BLINKY_CTRL_ENABLE_LED) && g_registers.data.blink_interval &&
      ((HAL_GetTick() - s_last_blink) >= g_registers.data.blink_interval)) {
    s_last_blink = HAL_GetTick();
    blinky_set_led(pfw, !s_led_on, 0);
  }

  // * * * Millisecond counter limit (alarm bit 7) * * *
  if (g_counter_wrapped) {
    g_counter_wrapped = 0;
    pfw->alarm_status |= BLINKY_ALARM_COUNTER_LIMIT;
  }

  // * * * Save changed registers * * *
  if (s_save_pending && ((HAL_GetTick() - s_save_time) >= BLINKY_SAVE_DELAY_MS)) {
    blinky_save_now();
  }
}

///////////////////////////////////////////////////////////////////////////////
// blinky_save_now
//

void
blinky_save_now(void)
{
  if (!s_save_pending) {
    return;
  }
  s_save_pending = 0;
  watchdog_feed(); // An erase of the sector (when it is full) takes about a second
  if (HAL_OK != flash_storage_save(g_registers.word_array, sizeof(register_union_t) / sizeof(uint16_t))) {
    LOGSTR("Failed to save the registers to flash\r\n");
  }
}

// ****************************************************************************
//                        VSCP protocol callbacks
// ****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// update_persistent_storage(void)
//

static void
update_persistent_storage(void)
{
  // Saved from blinky_work() a little later, so a burst of writes is one flash record
  blinky_request_save();
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_get_ms
//

uint32_t
vscp_frmw2_callback_get_ms(vscp_frmw2_firmware_context_t *pctx)
{
  return HAL_GetTick();
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_get_timestamp
//

uint64_t
vscp_frmw2_callback_get_timestamp(vscp_frmw2_firmware_context_t *pctx)
{
  // We can't get true nanosecond resolution on this platform, so we just return microseconds * 1000 to get nanoseconds.
  // hires_ts_t t = hires_now();
  // return t.usec * 1000 + t.sub_ns / 1000;
  return nsec_now();
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_send_event
//
// Node sending event to client
//

int
vscp_frmw2_callback_send_event(vscp_frmw2_firmware_context_t *pctx, vscp_event_t *pev)
{
  // The fifo is defined in the context for the link protocol which
  // should be in the user data of the firmwarecontext.
  vscp_link_ctx_t *ctx_link = (vscp_link_ctx_t *) pctx->puserdata;

  // Do not allow write to fifo if we are not connected
  if (VSCP_STATE_DISCONNECTED == ctx_link->sock) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  // There is only one connection in this demo firmware, so we can directly write to its out queue.
  // In a more complex firmware, we would need to check which connections are open, and which
  // ones are validated, and write to all validated connections.

  for (int i = 0; i < BLINKY_MAX_TCP_CONNECTIONS; i++) {

    vscp_event_t *pnew = vscp_fwhlp_mkEventCopy(pev);
    if (NULL == pnew) {
      return VSCP_ERROR_MEMORY;
    }
    else {
      pnew->obid = pev->obid; // Keep the same obid as the original event

      if (vscp_fifo_write(&ctx_link->fifoEventsOut, pnew)) {
        ctx_link->statistics.cntTransmitFrames++;
        ctx_link->statistics.cntTransmitData += pnew->sizeData;
      }
      else {
        ctx_link->statistics.cntOverruns++;
        vscp_fwhlp_deleteEvent(&pnew);
      }
    }
  }

  // NOTE: Do NOT call vscp_fwhlp_deleteEvent(&pev) here.
  // The send_event contract states "the event is copied by the callback" —
  // the caller (vscp-firmware-level2.c) owns the event struct and its pdata
  // and is responsible for freeing them. pev points to the caller's
  // stack-allocated vscp_event_t, so free()ing it is undefined behaviour
  // and causes an immediate HardFault on Cortex-M.

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_dm_action
//

/*!
  Actions are defined in the VSCP DM specification. The action parameter is a uint8_t
  that indicates which action to perform, and pparam is a pointer to any additional
  parameters needed for the action. The callback should return VSCP_ERROR_SUCCESS if
  the action was performed successfully, or an appropriate error code if not.

  Actions
  -------
  0 - NOOP -  No operation, do nothing
  1 - SET -  Set LED state to ON or OFF based on pparam[0] (0=OFF, 1=ON)
  2 - ON -  Turn LED ON
  3 - OFF -  Turn LED OFF
  4 - TOGGLE -  Toggle LED state
  5 - START BLINK -  Start blinking LED with interval specified in pparam[0] (in ms)
  6 - STOP BLINK -  Stop blinking LED and restore to previous state
  7 - CLEAR -  Clear the millisecond counter

  # defines are in blinky.h
*/

int
vscp_frmw2_callback_dm_action(vscp_frmw2_firmware_context_t *pctx,
                              const vscp_event_t *pev,
                              uint8_t action,
                              const uint8_t *pparam)
{
  const uint8_t param = (NULL != pparam) ? pparam[0] : 0;

  if (BLINKY_ACTION_CLR_COUNTER == action) {
    g_user_reg_millisecond = 0;
    return VSCP_ERROR_SUCCESS;
  }

  // The LED actions work when the LED is enabled in the control register
  if (!(g_registers.data.control & BLINKY_CTRL_ENABLE_LED)) {
    return VSCP_ERROR_SUCCESS;
  }

  switch (action) {
    case BLINKY_ACTION_NOOP:
      break;

    case BLINKY_ACTION_CTRL_LED_STATE:
      s_blinking = 0;
      if (BLINKY_ARG_CTRL_LED_TOGGLE == param) {
        blinky_set_led(pctx, !s_led_on, 1);
      }
      else {
        blinky_set_led(pctx, (BLINKY_ARG_CTRL_LED_OFF != param), 1);
      }
      break;

    case BLINKY_ACTION_CTRL_LED_ON:
      s_blinking = 0;
      blinky_set_led(pctx, 1, 1);
      break;

    case BLINKY_ACTION_CTRL_LED_OFF:
      s_blinking = 0;
      blinky_set_led(pctx, 0, 1);
      break;

    case BLINKY_ACTION_CTRL_LED_TOGGLE:
      s_blinking = 0;
      blinky_set_led(pctx, !s_led_on, 1);
      break;

    case BLINKY_ACTION_CTRL_LED_START_BLINK:
      // Blink with the interval in registers 4-5 (an interval of zero does not blink)
      s_blinking   = (0 != g_registers.data.blink_interval);
      s_last_blink = HAL_GetTick();
      break;

    case BLINKY_ACTION_CTRL_LED_STOP_BLINK:
      s_blinking = 0;
      break;

    default:
      return VSCP_ERROR_PARAMETER;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_segment_ctrl_heartbeat
//

int
vscp_frmw2_callback_segment_ctrl_heartbeat(vscp_frmw2_firmware_context_t *pctx, uint16_t segcrc, uint32_t time)
{
  // We are not segment controller, so we just return success to keep the client happy
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_report_events_of_interest
//

int
vscp_frmw2_callback_report_events_of_interest(vscp_frmw2_firmware_context_t *pctx)
{
  // We want all events, so we just return a success code to keep the client happy
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_set_event_time
//

/*
  New version of the the firmware just use a 64 bit timestamp and not the time field in the event.

  If the system has a real time clock, the time field should be converted to a
  64 bit timestamp and stored in the event. The time field is then set to zero.
  If the timestamp is set to zero it will be set to current in the first communication
  interface it traverses.
  The vscp_fwhlp_to_unix_ns() function can be used to convert the time field to a 64 bit
  timestamp.
*/

int
vscp_frmw2_callback_set_event_time(vscp_frmw2_firmware_context_t *pctx, vscp_event_t *const pev)
{
  if (NULL == pev) {
    return VSCP_ERROR_INVALID_POINTER;
  }
  // Always set 64-bit nanosecond timestamp
  pev->head |= VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  pev->year         = 0xffff;
  pev->month        = 0xff;
  pev->timestamp_ns = vscp_frmw2_callback_get_timestamp(pctx);
  return VSCP_ERROR_SUCCESS;
}

// Helper function to convert IP address string to byte array
// Returns 0 on success, -1 on invalid input.
// out[] must have space for 4 bytes.

static int
ip_str_to_bytes(uint8_t out[4], const char *ip_str)
{
  int octet  = 0;
  int value  = 0;
  int digits = 0;

  for (const char *p = ip_str;; p++) {
    char c = *p;

    if (c >= '0' && c <= '9') {
      value = value * 10 + (c - '0');
      digits++;
      if (value > 255 || digits > 3) {
        return -1; // octet out of range or too many digits
      }
    }
    else if (c == '.' || c == '\0') {
      if (digits == 0 || octet >= 4) {
        return -1; // empty octet, or already have 4 octets
      }
      out[octet++] = (uint8_t) value;
      value        = 0;
      digits       = 0;
      if (c == '\0') {
        break;
      }
    }
    else {
      return -1; // invalid character
    }
  }

  if (octet != 4) {
    return -1; // not enough octets
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_get_ip_addr
//

int
vscp_frmw2_callback_get_ip_addr(vscp_frmw2_firmware_context_t *pctx, uint8_t *ip, uint8_t size)
{
  if (NULL == ip) {
    return VSCP_ERROR_INVALID_POINTER;
  }
  else {
    if (0 != ip_str_to_bytes(ip, WIZ_IP20_IP + 2)) { // Skip the "LI" prefix
      return VSCP_ERROR_PARAMETER;
    }
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_read_user_reg
//

int
vscp_frmw2_callback_read_user_reg(vscp_frmw2_firmware_context_t *pctx, uint16_t page, uint32_t reg, uint8_t *pval)
{
  if (NULL == pval) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  // All user registers are on page 0
  if (0 != page) {
    return VSCP_ERROR_PARAMETER;
  }

  if (BLINKY_REG_DEVICE_ZONE == reg) {
    *pval = g_registers.data.zone;
  }
  else if (BLINKY_REG_DEVICE_SUBZONE == reg) {
    *pval = g_registers.data.subzone;
  }
  else if (BLINKY_REG_DEVICE_STATUS == reg) {
    *pval = g_user_reg_status; // Non persistent
  }
  else if (BLINKY_REG_DEVICE_CONTROL == reg) {
    *pval = g_registers.data.control;
  }
  else if (BLINKY_REG_DEVICE_BLINK_INTERVAL_MSB == reg) {
    *pval = (g_registers.data.blink_interval >> 8) & 0xff;
  }
  else if (BLINKY_REG_DEVICE_BLINK_INTERVAL_LSB == reg) {
    *pval = g_registers.data.blink_interval & 0xff;
  }
  else if (BLINKY_REG_DEVICE_COUNTER_0 == reg) {
    *pval = (g_user_reg_millisecond >> 24) & 0xff;
  }
  else if (BLINKY_REG_DEVICE_COUNTER_1 == reg) {
    *pval = (g_user_reg_millisecond >> 16) & 0xff;
  }
  else if (BLINKY_REG_DEVICE_COUNTER_2 == reg) {
    *pval = (g_user_reg_millisecond >> 8) & 0xff;
  }
  else if (BLINKY_REG_DEVICE_COUNTER_3 == reg) {
    *pval = g_user_reg_millisecond & 0xff;
  }
  else if (BLINKY_REG_DEVICE_BUTTON_BYTE0 == reg) {
    *pval = g_registers.data.button_zero_opt_byte;
  }
  else if (BLINKY_REG_DEVICE_BUTTON_ZONE == reg) {
    *pval = g_registers.data.button_zone;
  }
  else if (BLINKY_REG_DEVICE_BUTTON_SUBZONE == reg) {
    *pval = g_registers.data.button_subzone;
  }
  else if ((reg >= BLINKY_REG_DM_START) && (reg < BLINKY_REG_DM_START + sizeof(g_registers.data.dm))) {
    *pval = g_registers.data.dm[reg - BLINKY_REG_DM_START];
  }
  else {
    return VSCP_ERROR_PARAMETER;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_write_user_reg
//

int
vscp_frmw2_callback_write_user_reg(vscp_frmw2_firmware_context_t *pctx, uint16_t page, uint32_t reg, uint8_t val)
{
  // All user registers are on page 0
  if (0 != page) {
    return VSCP_ERROR_PARAMETER;
  }

  if (BLINKY_REG_DEVICE_ZONE == reg) {
    g_registers.data.zone = val;
    pctx->zone            = val; // Used by the decision matrix zone match
    update_persistent_storage();
  }
  else if (BLINKY_REG_DEVICE_SUBZONE == reg) {
    g_registers.data.subzone = val;
    pctx->subzone            = val;
    update_persistent_storage();
  }
  else if (BLINKY_REG_DEVICE_STATUS == reg) {
    // Bit 1 turns the LED on/off (when the LED is enabled); the button bit is read only
    if (g_registers.data.control & BLINKY_CTRL_ENABLE_LED) {
      s_blinking = 0;
      blinky_set_led(pctx, (val & BLINKY_STATUS_LED_ON) ? 1 : 0, 1);
    }
  }
  else if (BLINKY_REG_DEVICE_CONTROL == reg) {
    uint8_t old              = g_registers.data.control;
    g_registers.data.control = val;
    // Enabling the LED starts blinking (if the interval is set), disabling stops it
    if (!(old & BLINKY_CTRL_ENABLE_LED) && (val & BLINKY_CTRL_ENABLE_LED)) {
      s_blinking   = (0 != g_registers.data.blink_interval);
      s_last_blink = HAL_GetTick();
    }
    else if (!(val & BLINKY_CTRL_ENABLE_LED)) {
      s_blinking = 0;
    }
    update_persistent_storage();
  }
  else if (BLINKY_REG_DEVICE_BLINK_INTERVAL_MSB == reg) {
    g_registers.data.blink_interval = (g_registers.data.blink_interval & 0x00ff) | ((uint16_t) val << 8);
    s_blinking                      = (0 != g_registers.data.blink_interval); // Zero stops blinking
    update_persistent_storage();
  }
  else if (BLINKY_REG_DEVICE_BLINK_INTERVAL_LSB == reg) {
    g_registers.data.blink_interval = (g_registers.data.blink_interval & 0xff00) | val;
    s_blinking                      = (0 != g_registers.data.blink_interval);
    update_persistent_storage();
  }
  else if (BLINKY_REG_DEVICE_BUTTON_BYTE0 == reg) {
    g_registers.data.button_zero_opt_byte = val;
    update_persistent_storage();
  }
  else if (BLINKY_REG_DEVICE_BUTTON_ZONE == reg) {
    g_registers.data.button_zone = val;
    update_persistent_storage();
  }
  else if (BLINKY_REG_DEVICE_BUTTON_SUBZONE == reg) {
    g_registers.data.button_subzone = val;
    update_persistent_storage();
  }
  // Writing any of the counter registers clears the counter (not persistent)
  else if ((BLINKY_REG_DEVICE_COUNTER_0 <= reg) && (BLINKY_REG_DEVICE_COUNTER_3 >= reg)) {
    g_user_reg_millisecond = 0;
  }
  else if ((reg >= BLINKY_REG_DM_START) && (reg < BLINKY_REG_DM_START + sizeof(g_registers.data.dm))) {
    g_registers.data.dm[reg - BLINKY_REG_DM_START] = val;
    update_persistent_storage();
  }
  else {
    return VSCP_ERROR_PARAMETER;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_stdreg_change
//

int
vscp_frmw2_callback_stdreg_change(vscp_frmw2_firmware_context_t *pctx, uint32_t stdreg)
{
  // The firmware keeps the user id and nickname in its context; copy them
  // to the register storage that is saved in flash
  if (VSCP_STD_REGISTER_USER_ID == stdreg) {
    memcpy(g_registers.data.userdata, pctx->userId, sizeof(g_registers.data.userdata));
    update_persistent_storage();
  }
  else if ((VSCP_STD_REGISTER_NICKNAME_ID_LSB == stdreg) || (VSCP_STD_REGISTER_NICKNAME_ID_MSB == stdreg)) {
    g_registers.data.nickname = pctx->nickname;
    update_persistent_storage();
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_restore_defaults
//

int
vscp_frmw2_callback_restore_defaults(vscp_frmw2_firmware_context_t *pctx)
{
  resetRegisters();
  pctx->zone     = g_registers.data.zone;
  pctx->subzone  = g_registers.data.subzone;
  pctx->nickname = g_registers.data.nickname;
  memcpy(pctx->userId, g_registers.data.userdata, sizeof(g_registers.data.userdata));
  blinky_init(pctx);
  update_persistent_storage();
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_get_guid
//

const uint8_t *
vscp_frmw2_callback_get_guid(vscp_frmw2_firmware_context_t *pctx)
{
  return pctx->guid;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_enter_bootloader
//

void
vscp_frmw2_callback_enter_bootloader(vscp_frmw2_firmware_context_t *pctx)
{
  // No bootloader support in this demo firmware
  return;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_reset
//

int
vscp_frmw2_callback_reset(vscp_frmw2_firmware_context_t *pctx)
{
  // Reset the device. This is a placeholder for the actual reset logic,
  // which may involve setting a flag or calling a system reset function.

  NVIC_SystemReset();
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_feed_watchdog
//

int
vscp_frmw2_callback_feed_watchdog(vscp_frmw2_firmware_context_t *pctx)
{
  watchdog_feed();
  return VSCP_ERROR_SUCCESS;
}
