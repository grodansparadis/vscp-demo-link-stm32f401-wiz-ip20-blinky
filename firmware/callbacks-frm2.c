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

extern register_union_t g_registers;             // Global register storage
extern volatile uint32_t g_user_reg_millisecond; // Millisecond counter register, updated every 1 ms
extern volatile uint8_t g_user_reg_status;       // Status register, non-persistent

// ****************************************************************************
//                        VSCP protocol callbacks
// ****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// update_persistent_storage(void)
//

static void
update_persistent_storage(void)
{
  if (HAL_OK != flash_storage_write(FLASH_STORAGE_DATA_OFFSET,
                                    g_registers.word_array,
                                    sizeof(register_union_t) / sizeof(uint16_t))) {
    LOGSTR("Failed to write flash storage\r\n");
  }
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
  switch (action) {
    case BLINKY_ACTION_NOOP:
      // Do nothing
      break;

    case BLINKY_ACTION_CTRL_LED_STATE:
      if (pparam[0] == BLINKY_ARG_CTRL_LED_OFF) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        g_user_reg_status &= ~BLINKY_STATUS_LED_ON;
      }
      else if (pparam[0] == BLINKY_ARG_CTRL_LED_ON) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        g_user_reg_status |= BLINKY_STATUS_LED_ON;
      }
      else if (pparam[0] == BLINKY_ARG_CTRL_LED_TOGGLE) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        g_user_reg_status ^= BLINKY_STATUS_LED_ON;
      }
      break;

    case BLINKY_ACTION_CTRL_LED_ON:
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
      g_user_reg_status |= BLINKY_STATUS_LED_ON;
      break;

    case BLINKY_ACTION_CTRL_LED_OFF:
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
      g_user_reg_status &= ~BLINKY_STATUS_LED_ON;
      break;

    case BLINKY_ACTION_CTRL_LED_TOGGLE:
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      g_user_reg_status ^= BLINKY_STATUS_LED_ON;
      break;

    case BLINKY_ACTION_CTRL_LED_START_BLINK:
      // Start blinking with interval specified in pparam[0] and pparam[1]
      g_registers.data.blink_interval = ((uint16_t) pparam[0] << 8) | pparam[1];
      g_registers.data.control |= BLINKY_CTRL_ENABLE_LED;
      update_persistent_storage();
      break;

    case BLINKY_ACTION_CTRL_LED_STOP_BLINK:
      // Stop blinking
      g_registers.data.control &= ~BLINKY_CTRL_ENABLE_LED;
      update_persistent_storage();
      break;

    case BLINKY_ACTION_CLR_COUNTER:
      g_user_reg_millisecond = 0;
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
  // We just have one page so we ignore the page parameter. In a more complex device, you would handle different pages
  // here.

  // Check pointers (pdata allowed to be NULL)
  if (NULL == pval) {
    return VSCP_ERROR_INVALID_POINTER;
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
    *pval = g_registers.data.blink_interval >> 8 & 0xff;
  }
  else if (BLINKY_REG_DEVICE_BLINK_INTERVAL_LSB == reg) {
    *pval = g_registers.data.blink_interval & 0xff;
  }
  else if (BLINKY_REG_DEVICE_COUNTER_0 == reg) {
    *pval = g_user_reg_millisecond >> 24 & 0xff;
  }
  else if (BLINKY_REG_DEVICE_COUNTER_1 == reg) {
    *pval = g_user_reg_millisecond >> 16 & 0xff;
  }
  else if (BLINKY_REG_DEVICE_COUNTER_2 == reg) {
    *pval = g_user_reg_millisecond >> 8 & 0xff;
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
  else {
    // Invalid register
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
  // We just have one page so we ignore the page parameter. In a more complex device, you would handle different pages
  // here.

  if (BLINKY_REG_DEVICE_ZONE == reg) {
    g_registers.data.zone = val;
    update_persistent_storage();
  }
  else if (BLINKY_REG_DEVICE_SUBZONE == reg) {
    g_registers.data.subzone = val;
    update_persistent_storage();
  }
  else if (BLINKY_REG_DEVICE_STATUS == reg) {
    // Only the LD can be set/reset
    g_user_reg_status |= val & BLINKY_STATUS_LED_ON; // Non persistent
    // Update the LED state based on the status register
    if (g_user_reg_status & BLINKY_STATUS_LED_ON) {
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); // Turn on LED
    }
    else {
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); // Turn off LED
    }
  }
  else if (BLINKY_REG_DEVICE_CONTROL == reg) {
    g_registers.data.control = val;
    update_persistent_storage();
  }
  else if (BLINKY_REG_DEVICE_BLINK_INTERVAL_MSB == reg) {
    g_registers.data.blink_interval = (g_registers.data.blink_interval & 0x00ff) | ((uint16_t) val << 8);
    update_persistent_storage();
  }
  else if (BLINKY_REG_DEVICE_BLINK_INTERVAL_LSB == reg) {
    g_registers.data.blink_interval = (g_registers.data.blink_interval & 0xff00) | val;
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
  // Writing to the counter reset it
  else if (BLINKY_REG_DEVICE_COUNTER_0 == reg || BLINKY_REG_DEVICE_COUNTER_1 == reg || BLINKY_REG_DEVICE_COUNTER_2 == reg ||
           BLINKY_REG_DEVICE_COUNTER_3 == reg) {
    g_user_reg_millisecond = 0; // Reset the millisecond counter register;
    update_persistent_storage();
  }
  else {
    // Invalid register
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
  if (VSCP_STD_REGISTER_USER_ID == stdreg) {
    // Update persistent values
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
