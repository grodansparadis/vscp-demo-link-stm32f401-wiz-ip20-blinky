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

#include <vscp-fifo.h>

#include "vscp-link-protocol.h"
#include "vscp-firmware-helper.h"
#include "vscp-firmware-level2.h"

#include "main.h"
#include "blinky.h"
#include "regdefs.h"

extern char g_ipaddrstr[20];
extern char g_macaddrstr[20];
extern struct _eeprom_ eeprom;

// ****************************************************************************
//                        VSCP protocol callbacks
// ****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_get_ms
//

uint32_t
vscp_frmw2_callback_get_ms(vscp_frmw2_firmware_context_t *pctx)
{
  return HAL_GetTick();
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_get_guid
//

const uint8_t *
vscp_frmw2_callback_get_guid(vscp_frmw2_firmware_context_t *pctx)
{
  return pctx->guid;
}

#ifdef THIS_FIRMWARE_ENABLE_WRITE_2PROTECTED_LOCATIONS

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_write_manufacturer_id
//

int
vscp_frmw2_callback_write_manufacturer_id(vscp_frmw2_firmware_context_t *pctx, uint8_t pos, uint8_t val)
{
  if (pos < 4) {
    // TODO // TODO eeprom_write(&eeprom, STDREG_MANUFACTURER_ID0 + pos, val);
  }
  else if (pos < 8) {
    // TODO // TODO eeprom_write(&eeprom, STDREG_MANUFACTURER_SUBID0 + pos - 4, val);
  }

  // Commit changes to 'eeprom'
  // TODO eeprom_commit(&eeprom);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_write_guid
//

int
vscp_frmw2_callback_write_guid(vscp_frmw2_firmware_context_t *pctx, uint8_t pos, uint8_t val)
{
  // TODO eeprom_write(&eeprom, STDREG_GUID0 + pos, val);

  // Commit changes to 'eeprom'
  // TODO eeprom_commit(&eeprom);

  return VSCP_ERROR_SUCCESS;
}

#endif // THIS_FIRMWARE_ENABLE_WRITE_2PROTECTED_LOCATIONS

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_read_user_reg
//

int
vscp_frmw2_callback_read_user_reg(vscp_frmw2_firmware_context_t *pctx, uint32_t reg, uint8_t *pval)
{
  // Check pointers (pdata allowed to be NULL)
  if (NULL == pval) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if (REG_DEVICE_ZONE == reg) {
    *pval = 0; // TODO  eeprom_read(&eeprom, REG_DEVICE_ZONE);
  }
  else if (REG_DEVICE_SUBZONE == reg) {
    *pval = 0; // TODO  eeprom_read(&eeprom, REG_DEVICE_SUBZONE);
  }
  /*
  else if (REG_LED_CTRL == reg) {
    *pval = 0; // TODO  eeprom_read(&eeprom, REG_LED_CTRL);
  }
  else if (REG_LED_STATUS == reg) {
    *pval = 0; // TODO gpio_get(LED_PIN);
  }
  else if (REG_LED_BLINK_INTERVAL == reg) {
    *pval = 0; // TODO  eeprom_read(&eeprom, REG_LED_BLINK_INTERVAL);
  }
  else if (REG_IO_CTRL1 == reg) {
    *pval = 0; // TODO  eeprom_read(&eeprom, REG_IO_CTRL1);
  }
  else if (REG_IO_CTRL2 == reg) {
    *pval = 0; // TODO  eeprom_read(&eeprom, REG_IO_CTRL2);
  }
  else if (REG_IO_STATUS == reg) {
    uint32_t all = 0; // TODO  gpio_get_all();
    *pval        = (all >> 2) & 0xff;
  }
  else if (REG_TEMP_CTRL == reg) {
    *pval = 0; // TODO  eeprom_read(&eeprom, REG_TEMP_CTRL);
  }
  else if (REG_TEMP_RAW_MSB == reg) {
    float temp = read_onboard_temperature();
    *pval      = (((uint16_t) (100 * temp)) >> 8) & 0xff;
  }
  else if (REG_TEMP_RAW_LSB == reg) {
    float temp = read_onboard_temperature();
    *pval      = ((uint16_t) (100 * temp)) & 0xff;
  }
  else if (REG_TEMP_CORR_MSB == reg) {
    *pval = 0; // TODO  eeprom_read(&eeprom, REG_TEMP_CORR_MSB);
  }
  else if (REG_TEMP_CORR_LSB == reg) {
    *pval = 0; // TODO  eeprom_read(&eeprom, REG_TEMP_CORR_LSB);
  }
  else if (REG_TEMP_INTERVAL == reg) {
    *pval = 0; // TODO  eeprom_read(&eeprom, REG_TEMP_INTERVAL);
  }
  else if (REG_ADC0_CTRL == reg) {
    *pval = 0; // TODO  eeprom_read(&eeprom, REG_ADC0_CTRL);
  }
  else if (REG_ADC0_MSB == reg) {
    float adc = 0; // TODO 0; // TODO read_adc(0);
    *pval     = (((uint16_t) (100 * adc)) >> 8) & 0xff;
  }
  else if (REG_ADC0_LSB == reg) {
    float adc = 0; // TODO 0; // TODO read_adc(0);
    *pval     = ((uint16_t) (100 * adc)) & 0xff;
  }
  else if (REG_ADC1_CTRL == reg) {
    *pval = 0; // TODO  eeprom_read(&eeprom, REG_ADC0_CTRL);
  }
  else if (REG_ADC1_MSB == reg) {
    float adc = 0; // TODO read_adc(1);
    *pval     = (((uint16_t) (100 * adc)) >> 8) & 0xff;
  }
  else if (REG_ADC1_LSB == reg) {
    float adc = 0; // TODO read_adc(1);
    *pval     = ((uint16_t) (100 * adc)) & 0xff;
  }
  else if (REG_ADC2_CTRL == reg) {
    *pval = 0; // TODO  eeprom_read(&eeprom, REG_ADC0_CTRL);
  }
  else if (REG_ADC2_MSB == reg) {
    float adc = 0; // TODO 0; // TODO read_adc(2);
    *pval     = (((uint16_t) (100 * adc)) >> 8) & 0xff;
  }
  else if (REG_ADC2_LSB == reg) {
    float adc = 0; // TODO 0; // TODO read_adc(2);
    *pval     = ((uint16_t) (100 * adc)) & 0xff;
  }
  else if ((REG_BOARD_ID0 >= reg) && (REG_BOARD_ID8 <= reg)) {
    // TODO
    // pico_unique_board_id_t boardid;
    // pico_get_unique_board_id(&boardid);
    // *pval = boardid.id[reg - REG_BOARD_ID0];
  }
    */
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
vscp_frmw2_callback_write_user_reg(vscp_frmw2_firmware_context_t *pctx, uint32_t reg, uint8_t val)
{
  if (REG_DEVICE_ZONE == reg) {
    // TODO eeprom_write(&eeprom, REG_DEVICE_ZONE, val);
  }
  else if (REG_DEVICE_SUBZONE == reg) {
    // TODO eeprom_write(&eeprom, REG_DEVICE_SUBZONE, val);
  }
  /*
  else if (REG_LED_CTRL == reg) {
    // TODO eeprom_write(&eeprom, REG_LED_CTRL, val);
  }
  else if (REG_LED_STATUS == reg) {
    if (val) {
      // TODO gpio_put(LED_PIN, 1);
    }
    else {
      // TODO gpio_put(LED_PIN, 0);
    }
  }
  else if (REG_LED_BLINK_INTERVAL == reg) {
    // TODO eeprom_write(&eeprom, REG_LED_BLINK_INTERVAL, val);
  }
  else if (REG_IO_CTRL1 == reg) {
    // TODO eeprom_write(&eeprom, REG_IO_CTRL1, val);
  }
  else if (REG_IO_CTRL2 == reg) {
    // TODO eeprom_write(&eeprom, REG_IO_CTRL2, val);
  }
  else if (REG_IO_STATUS == reg) {
  }
  else if (REG_TEMP_CTRL == reg) {
    // TODO eeprom_write(&eeprom, REG_TEMP_CTRL, val);
  }
  else if (REG_TEMP_CORR_MSB == reg) {
    // TODO eeprom_write(&eeprom, REG_TEMP_CORR_MSB, val);
  }
  else if (REG_TEMP_CORR_LSB == reg) {
    // TODO eeprom_write(&eeprom, REG_TEMP_CORR_LSB, val);
  }
  else if (REG_TEMP_INTERVAL == reg) {
    // TODO eeprom_write(&eeprom, REG_TEMP_INTERVAL, val);
  }
  else if (REG_ADC0_CTRL == reg) {
    // TODO eeprom_write(&eeprom, REG_ADC0_CTRL, val);
  }
  else if (REG_ADC1_CTRL == reg) {
    // TODO eeprom_write(&eeprom, REG_ADC1_CTRL, val);
  }
  else if (REG_ADC2_CTRL == reg) {
    // TODO eeprom_write(&eeprom, REG_ADC2_CTRL, val);
  }*/
  else {
    return VSCP_ERROR_PARAMETER;
  }

  // Commit changes to 'eeprom'
  // TODO eeprom_commit(&eeprom);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_stdreg_change
//

int
vscp_frmw2_callback_stdreg_change(vscp_frmw2_firmware_context_t *pctx, uint32_t stdreg)
{
  if (VSCP_STD_REGISTER_USER_ID == stdreg) {
    // TODO eeprom_write(&eeprom, STDREG_USER_ID0, pctx->userId[0]);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_enter_bootloader
//

void
vscp_frmw2_callback_enter_bootloader(vscp_frmw2_firmware_context_t *pctx)
{
  // TODO implement if bootloader entry is supported by this firmware
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_report_dmatrix
//

int
vscp_frmw2_callback_report_dmatrix(vscp_frmw2_firmware_context_t *pctx)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_report_mdf
//

int
vscp_frmw2_callback_report_mdf(vscp_frmw2_firmware_context_t *pctx)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_report_events_of_interest
//

int
vscp_frmw2_callback_report_events_of_interest(vscp_frmw2_firmware_context_t *pctx)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_get_timestamp
//

uint64_t
vscp_frmw2_callback_get_timestamp(vscp_frmw2_firmware_context_t *pctx)
{
  return usec_now();
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_get_time
//

int
vscp_frmw2_callback_get_time(vscp_frmw2_firmware_context_t *pctx, const vscp_event_ex_t *pex)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_send_event
//

int
vscp_frmw2_callback_send_event(vscp_frmw2_firmware_context_t *pctx, vscp_event_t *pev)
{
  // There is only one connection in this demo firmware, so we can directly write to its out queue.
  // In a more complex firmware, we would need to check which connections are open, and which
  // ones are validated, and write to all validated connections.

  for (int i = 0; i < BLINKY_MAX_TCP_CONNECTIONS; i++) {

    vscp_event_t *pnew = vscp_fwhlp_mkEventCopy(pev);
    if (NULL == pnew) {
      return VSCP_ERROR_MEMORY;
    }
    else {
      pnew->obid = 0xffffffff; // The device
      // The fifo is defined in the context for the link protocol which should be in the user data of the firmware
      // context.
      vscp_link_ctx_t *ctx_link = (vscp_link_ctx_t *) pctx->puserdata;
      if (vscp_fifo_write(&ctx_link->fifoEventsOut, pnew)) {
        LOGSTR("Event written to fifo\n");
      }
      else {
        LOGSTR("Failed to write event to fifo\n");
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
// vscp_frmw2_callback_restore_defaults
//

int
vscp_frmw2_callback_restore_defaults(vscp_frmw2_firmware_context_t *pctx)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_write_user_id
//

int
vscp_frmw2_callback_write_user_id(vscp_frmw2_firmware_context_t *pctx, uint8_t pos, uint8_t val)
{
  // TODO // TODO eeprom_write(&eeprom, STDREG_USER_ID0 + pos, val);

  // Commit changes to 'eeprom'
  // TODO eeprom_commit(&eeprom);

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
    return VSCP_ERROR_PARAMETER;
  }
  else {
    if (0 != ip_str_to_bytes(ip, g_ipaddrstr)) {
      return VSCP_ERROR_PARAMETER;
    }
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_set_event_time
//

/*
  New version of the the firmware just use a 64 bit timestamp and not the time field in the event.
*/

int
vscp_frmw2_callback_set_event_time(vscp_frmw2_firmware_context_t *pctx, vscp_event_t *const pev)
{
  if (NULL == pev) {
    return VSCP_ERROR_PARAMETER;
  }

  pev->timestamp = vscp_frmw2_callback_get_timestamp(pctx);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_reset
//

int
vscp_frmw2_callback_reset(vscp_frmw2_firmware_context_t *pctx)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_feed_watchdog
//

int
vscp_frmw2_callback_feed_watchdog(vscp_frmw2_firmware_context_t *pctx)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_dm_action
//

int
vscp_frmw2_callback_dm_action(vscp_frmw2_firmware_context_t *pctx, const vscp_event_t *pev, uint8_t action, const uint8_t *pparam)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_segment_ctrl_heartbeat
//

int
vscp_frmw2_callback_segment_ctrl_heartbeat(vscp_frmw2_firmware_context_t *pctx, uint16_t segcrc, uint32_t time)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_read_reg
//

int
vscp_frmw2_callback_read_reg(vscp_frmw2_firmware_context_t *pctx, uint16_t page, uint32_t reg, uint8_t *pval)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_write_reg
//

int
vscp_frmw2_callback_write_reg(vscp_frmw2_firmware_context_t *pctx, uint16_t page, uint32_t reg, uint8_t val)
{
  return VSCP_ERROR_SUCCESS;
}

// ----------------------------------------------------------------------------

#ifdef THIS_FIRMWARE_VSCP_DISCOVER_SERVER

///////////////////////////////////////////////////////////////////////////////
// vscp_frmw2_callback_high_end_server_response
//

int
vscp_frmw2_callback_high_end_server_response(vscp_frmw2_firmware_context_t *pctx)
{
  return VSCP_ERROR_SUCCESS;
}

#endif // THIS_FIRMWARE_VSCP_DISCOVER_SERVER

// ----------------------------------------------------------------------------