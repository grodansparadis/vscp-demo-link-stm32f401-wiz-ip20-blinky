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

#include <vscp-fifo.h>

#include "vscp-link-protocol.h"
#include "vscp-firmware-helper.h"
#include "vscp-firmware-level2.h"

#include "main.h"
#include "blinky.h"
#include "regdefs.h"
#include "vscp.h"

extern char g_ipaddrstr[20];
extern char g_macaddrstr[20];
extern register_union_t g_registers; // Global register storage
extern volatile uint32_t g_user_reg_millisecond; // Millisecond counter register, updated every 1 ms
extern volatile uint8_t g_user_reg_status; // Status register, non-persistent 

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
  // On devices that allow it, write the GUID to a write-protected area of memory
  // (e.g., EEPROM or flash).;

  return VSCP_ERROR_NOT_SUPPORTED;
}

#endif // THIS_FIRMWARE_ENABLE_WRITE_2PROTECTED_LOCATIONS

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

  if (REG_DEVICE_ZONE == reg) {
    *pval = g_registers.data.zone;
  }
  else if (REG_DEVICE_SUBZONE == reg) {
    *pval = g_registers.data.subzone;
  }

  else if (REG_DEVICE_STATUS == reg) {
    *pval = g_user_reg_status; // Non persistent
  }
  else if (REG_DEVICE_CONTROL == reg) {
    *pval = g_registers.data.control;
  }
  else if (REG_DEVICE_BLINK_INTERVAL_MSB == reg) {
    *pval = g_registers.data.blink_interval >> 8 & 0xff;
  }
  else if (REG_DEVICE_BLINK_INTERVAL_LSB == reg) {
    *pval = g_registers.data.blink_interval & 0xff;
  }
  else if (REG_DEVICE_COUNTER_0 == reg) {
    *pval = g_user_reg_millisecond >> 24 & 0xff;
  }
  else if (REG_DEVICE_COUNTER_1 == reg) {
    *pval = g_user_reg_millisecond >> 16 & 0xff;
  }
  else if (REG_DEVICE_COUNTER_2 == reg) {
    *pval = g_user_reg_millisecond >> 8 & 0xff;
  }
  else if (REG_DEVICE_COUNTER_3 == reg) {
    *pval = g_user_reg_millisecond & 0xff;
  }
  else if (REG_DEVICE_BUTTON_BYTE0 == reg) {
    *pval = g_registers.data.button_zero_opt_byte;
  }
  else if (REG_DEVICE_BUTTON_ZONE == reg) {
    *pval = g_registers.data.button_zone;
  }
  else if (REG_DEVICE_BUTTON_SUBZONE == reg) {
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

  if (REG_DEVICE_ZONE == reg) {
    g_registers.data.zone = val;
    update_persistent_storage();
  }
  else if (REG_DEVICE_SUBZONE == reg) {
    g_registers.data.subzone = val;
    update_persistent_storage();
  }
  else if (REG_DEVICE_STATUS == reg) {
    g_user_reg_status = val; // Non persistent
  }
  else if (REG_DEVICE_CONTROL == reg) {
    g_registers.data.control = val;
    update_persistent_storage();
  }
  else if (REG_DEVICE_BLINK_INTERVAL_MSB == reg) {
    g_registers.data.blink_interval = (g_registers.data.blink_interval & 0x00ff) | ((uint16_t) val << 8);
    update_persistent_storage();
  }
  else if (REG_DEVICE_BLINK_INTERVAL_LSB == reg) {
    g_registers.data.blink_interval = (g_registers.data.blink_interval & 0xff00) | val;
    update_persistent_storage();
  }
  else if (REG_DEVICE_BUTTON_BYTE0 == reg) {
    g_registers.data.button_zero_opt_byte = val;
    update_persistent_storage();
  }
  else if (REG_DEVICE_BUTTON_ZONE == reg) {
    g_registers.data.button_zone = val;
    update_persistent_storage();
  }
  else if (REG_DEVICE_BUTTON_SUBZONE == reg) {
    g_registers.data.button_subzone = val;
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
    return VSCP_ERROR_INVALID_POINTER;
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
    return VSCP_ERROR_INVALID_POINTER;
  }
  // Always set 64-bit nanosecond timestamp
  pev->head |= VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  pev->year         = 0xffff;
  pev->month        = 0xff;
  pev->timestamp_ns = vscp_frmw2_callback_get_timestamp(pctx) * 1000; // Convert microseconds to nanoseconds
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
vscp_frmw2_callback_dm_action(vscp_frmw2_firmware_context_t *pctx,
                              const vscp_event_t *pev,
                              uint8_t action,
                              const uint8_t *pparam)
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