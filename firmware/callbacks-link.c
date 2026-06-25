// FILE: callbacks-link.c

// This file holds callbacks for the VSCP tcp/ip link protocol

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

#include "vscp-compiler.h"
#include "vscp-projdefs.h"

#include <vscp.h>
#include <vscp-class.h>
#include <vscp-type.h>
#include <vscp-fifo.h>
#include <vscp-link-protocol.h>

#include "main.h"
#include "blinky.h"
#include "tim.h"
#include "usart.h"
#include "wiznet-ip20.h"
#include "build_number.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global stuff
extern node_persistent_config_t g_persistent;

// ****************************************************************************
//                       VSCP Link protocol callbacks
// ****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_write_client
//

int
vscp_link_callback_write_client(vscp_link_ctx_t *pctx, const char *msg)
{
  if ((NULL == pctx) || (NULL == msg)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  HAL_UART_Transmit(&huart1, (uint8_t *) msg, strlen(msg), HAL_MAX_DELAY);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_write_client
//

int
vscp_link_callback_welcome(vscp_link_ctx_t *pctx)
{
  if (NULL == pctx) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  char welcome_msg[256];
  snprintf(welcome_msg, sizeof(welcome_msg), BLINKY_WELCOME_MSG, THIS_FIRMWARE_BUILD_VERSION);

  vscp_link_callback_write_client(pctx, welcome_msg);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_quit
//

int
vscp_link_callback_quit(vscp_link_ctx_t *pctx)
{
  if (NULL == pctx) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  // Confirm quit
  vscp_link_callback_write_client(pctx, VSCP_LINK_MSG_GOODBYE);

  // Go to command mode
  int rv = wiznet_ip20_enter_command_mode();
  if (rv != 0) {
    LOGSTR("Failed to enter command mode\n");
  }

  // Restart ethernet module (will be ready for new connections again)
  wiznet_ip20_restart();
  resetLinkContextDefaults(pctx);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_help
//

int
vscp_link_callback_help(vscp_link_ctx_t *pctx, const char *arg)
{
  if ((NULL == pctx) || (NULL == arg)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  vscp_link_callback_write_client(pctx, VSCP_LINK_STD_HELP_TEXT);
  vscp_link_callback_write_client(pctx, VSCP_LINK_MSG_OK);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_get_interface_count
//

uint16_t
vscp_link_callback_get_interface_count(vscp_link_ctx_t *pctx)
{
  /* Return number of interfaces we support */
  return 1; // see vscp_link_callback_get_interface
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_get_interface
//

int
vscp_link_callback_get_interface(vscp_link_ctx_t *pctx, uint16_t index, struct vscp_interface_info *pif)
{
  if ((NULL == pctx) || (NULL == pif)) {
    return VSCP_ERROR_UNKNOWN_ITEM;
  }

  /*
    We have one interfaces on this device
    00:00 is the internal interface for the device itself.

    Each interfaces is returned as a comma separated string with the following format:

    'interface-id-n, type, interface-GUID-n, interface_real-name-n'

    interface types is in vscp.h
   */

  switch (index) {

    case 0:
      pif->idx  = index;
      pif->type = VSCP_INTERFACE_TYPE_LEVEL2DRV;
      memcpy(pif->guid, g_persistent.guid, 16);
      strncpy(pif->description, "Interface for the device itself", sizeof(pif->description));
      break;

      // case 1: {
      //   uint8_t guid[16];
      //   pif->idx  = index;
      //   pif->type = VSCP_INTERFACE_TYPE_LEVEL1DRV;
      //   memcpy(guid, g_persistent.guid, 16);
      //   guid[13] = 0x01; // Interface 0x0001
      //   memcpy(pif->guid, g_persistent.guid, 16);
      //   strncpy(pif->description, "Interface for the CAN4VSCP channel", sizeof(pif->description));
      // } break;

    default:
      return VSCP_ERROR_UNKNOWN_ITEM;
      break;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_close_interface
//

int
vscp_link_callback_close_interface(vscp_link_ctx_t *pctx, uint8_t *pguid)
{
  return VSCP_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_check_user
//

int
vscp_link_callback_check_user(vscp_link_ctx_t *pctx, const char *arg)
{
  if ((NULL == pctx) || (NULL == arg)) {
    LOGSTR("Invalid context pointer or user arg\n");
    return VSCP_ERROR_INVALID_POINTER;
  }

  // trim
  const char *p = arg;
  while (*p && isspace((unsigned char) *p)) {
    p++;
  }

  LOGSTR("Username: %s\n", p);

  // Save username in context for later password validation
  strncpy(pctx->user, p, VSCP_LINK_MAX_USER_NAME_LENGTH);
  LOGSTR("Username: %s\n", pctx->user);

  vscp_link_callback_write_client(pctx, VSCP_LINK_MSG_USENAME_OK);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_check_password
//

int
vscp_link_callback_check_password(vscp_link_ctx_t *pctx, const char *arg)
{
  if ((NULL == pctx) || (NULL == arg)) {
    LOGSTR("Invalid context pointer or password arg\n");
    return VSCP_ERROR_INVALID_POINTER;
  }

  // Must have a username before a password
  if (!strlen(pctx->user)) {
    vscp_link_callback_write_client(pctx, VSCP_LINK_MSG_NEED_USERNAME);
    LOGSTR("Password: No username yet\n");
    return VSCP_ERROR_CREDENTIALS;
  }

  // trim password
  const char *p = arg;
  while (*p && isspace((unsigned char) *p)) {
    p++;
  }

  LOGSTR("Username:'%s'\n", pctx->user);
  LOGSTR("Password '%s'\n", p);

  // if (0 == strcmp(pctx->user, "admin") && 0 == strcmp(p, "secret")) {

  if (validate_user(pctx->user, p)) {
    pctx->bValidated = true;
    pctx->privLevel  = 15;
  }
  else {
    pctx->user[0]    = '\0';
    pctx->bValidated = false;
    pctx->privLevel  = 0;
    vscp_link_callback_write_client(pctx, VSCP_LINK_MSG_PASSWORD_ERROR);
    LOGSTR("Credentials: Invalid\n");
    return VSCP_ERROR_CREDENTIALS;
  }

  vscp_link_callback_write_client(pctx, VSCP_LINK_MSG_PASSWORD_OK);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_challenge
//

int
vscp_link_callback_challenge(vscp_link_ctx_t *pctx, const char *arg)
{
  char buf[80];
  char random_data[32];
  if ((NULL == pctx) || (NULL == arg)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  const char *p = arg;
  while (*p && isspace((unsigned char) *p)) {
    p++;
  }

  static const char challenge_prefix[] = "+OK - ";
  size_t pos                           = (size_t) snprintf(buf, sizeof(buf), "%s", challenge_prefix);
  if (pos >= sizeof(buf)) {
    return VSCP_ERROR_MEMORY;
  }

  for (int i = 0; i < 32; i++) {
    random_data[i] = rand() >> 16;
    if (i < (int) (sizeof(challenge_prefix) - 1)) {
      random_data[i] += (uint8_t) challenge_prefix[i];
    }

    if ((pos + 2) >= sizeof(buf)) {
      return VSCP_ERROR_MEMORY;
    }

    vscp_fwhlp_dec2hex(random_data[i], &buf[pos], 2);
    pos += 2;
  }

  if ((pos + 2) >= sizeof(buf)) {
    return VSCP_ERROR_MEMORY;
  }

  buf[pos++] = '\r';
  buf[pos++] = '\n';
  buf[pos]   = '\0';

  vscp_link_callback_write_client(pctx, buf);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_check_authenticated
//

int
vscp_link_callback_check_authenticated(vscp_link_ctx_t *pctx)
{
  if (NULL == pctx) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if (pctx->bValidated) {
    return VSCP_ERROR_SUCCESS;
  }

  return VSCP_ERROR_INVALID_PERMISSION;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_check_privilege
//

int
vscp_link_callback_check_privilege(vscp_link_ctx_t *pctx, uint8_t priv)
{
  if (NULL == pctx) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if (pctx->privLevel >= priv) {
    return VSCP_ERROR_SUCCESS;
  }

  return VSCP_ERROR_INVALID_PERMISSION;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_test
//

int
vscp_link_callback_test(vscp_link_ctx_t *pctx, const char *arg)
{
  if ((NULL == pctx) || (NULL == arg)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  vscp_link_callback_write_client(pctx, VSCP_LINK_MSG_OK);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_send
//

/*
  A client sends an event. On some devices (gateways) this event
  will be forwarded on another interface (e.g. CAN, Modbus, etc). On other
  devices like this demo device, the event should be processed internally
  and not sent on any other interface.
*/

int
vscp_link_callback_send(vscp_link_ctx_t *pctx, vscp_event_t *pev)
{
  if ((NULL == pctx) || (NULL == pev)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  // If the event should be sent through another interface, we write 
  // the event to the fifo and process it later in the main loop. 
  // But since this demo firmware does not have any other interfaces, 
  // we can directly process the event here without going through the fifo. 
  // This is just to demonstrate how to send an event from the client and 
  // have it processed by the firmware.

  // if (!vscp_fifo_write(&pctx->fifoEventsOut, pev)) {
  //   LOGSTR("Outgoing fifo full, cannot accept event from client\n");
  //   return VSCP_ERROR_FIFO_FULL;
  // }

  // Update send statistics
  pctx->statistics.cntTransmitFrames++;
  pctx->statistics.cntTransmitData += pev->sizeData;

  // Forward event to VSCP stack for processing (e.g. to be sent on other interfaces, etc)
  vscp_frmw2_handle_protocol_event((vscp_frmw2_firmware_context_t *)pctx->user_data, pev);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_retr
//
// Client polls for an event.
//

int
vscp_link_callback_retr(vscp_link_ctx_t *pctx, vscp_event_t **ppev)
{

  if ((NULL == pctx) || (NULL == ppev)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  // Check if there is an event in the out queue
  if (!vscp_fifo_read(&pctx->fifoEventsOut, ppev)) {
    return VSCP_ERROR_RCV_EMPTY;
  }

  // We should have a valid pointer to an event here
  if (NULL == *ppev) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  // Update send statistics
  pctx->statistics.cntTransmitFrames++;
  pctx->statistics.cntTransmitData += (*ppev)->sizeData;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_enable_rcvloop
//

int
vscp_link_callback_enable_rcvloop(vscp_link_ctx_t *pctx, int bEnable)
{
  if (NULL == pctx) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  pctx->bRcvLoop          = bEnable;
  pctx->last_rcvloop_time = HAL_GetTick();

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_get_rcvloop_status
//

int
vscp_link_callback_get_rcvloop_status(vscp_link_ctx_t *pctx, int *pRcvLoop)
{
  if (NULL == pctx) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if (NULL == pRcvLoop) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  *pRcvLoop = pctx->bRcvLoop;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_chkData
//

int
vscp_link_callback_chkData(vscp_link_ctx_t *pctx, uint16_t *pcount)
{
  if (NULL == pctx) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  *pcount     = BLINKY_OUTGOING_FIFO_SIZE -vscp_fifo_getFree(&pctx->fifoEventsOut);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_clrall
//

int
vscp_link_callback_clrall(vscp_link_ctx_t *pctx)
{
  if (NULL == pctx) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  vscp_fifo_clear(&pctx->fifoEventsOut);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_get_channel_id
//

int
vscp_link_callback_get_channel_id(vscp_link_ctx_t *pctx, uint16_t *pchid)
{
  if ((NULL == pctx) || (NULL == pchid)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  *pchid = pctx->sock;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_get_guid
//

int
vscp_link_callback_get_guid(vscp_link_ctx_t *pctx, uint8_t *pguid)
{
  if ((NULL == pctx) || (NULL == pguid)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  memcpy(pguid, g_persistent.guid, 16);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_set_guid
//

int
vscp_link_callback_set_guid(vscp_link_ctx_t *pctx, uint8_t *pguid)
{
  if ((NULL == pctx) || (NULL == pguid)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  //memcpy(g_persistent.guid, pguid, 16);
  setGUID(pctx->guid);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_get_version
//

int
vscp_link_callback_get_version(vscp_link_ctx_t *pctx, uint8_t *pversion)
{
  if ((NULL == pctx) || (NULL == pversion)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  pversion[0] = THIS_FIRMWARE_MAJOR_VERSION;
  pversion[1] = THIS_FIRMWARE_MINOR_VERSION;
  pversion[2] = THIS_FIRMWARE_RELEASE_VERSION;
  pversion[3] = THIS_FIRMWARE_BUILD_VERSION;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_setFilter
//

int
vscp_link_callback_setFilter(vscp_link_ctx_t *pctx, vscpEventFilter *pfilter)
{
  if ((NULL == pctx) || (NULL == pfilter)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  pctx->filter.filter_class    = pfilter->filter_class;
  pctx->filter.filter_type     = pfilter->filter_type;
  pctx->filter.filter_priority = pfilter->filter_priority;
  memcpy(pctx->filter.filter_GUID, pfilter->filter_GUID, 16);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_setMask
//

int
vscp_link_callback_setMask(vscp_link_ctx_t *pctx, vscpEventFilter *pfilter)
{
  if ((NULL == pctx) || (NULL == pfilter)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  pctx->filter.mask_class    = pfilter->mask_class;
  pctx->filter.mask_type     = pfilter->mask_type;
  pctx->filter.mask_priority = pfilter->mask_priority;
  memcpy(pctx->filter.mask_GUID, pfilter->mask_GUID, 16);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_statistics
//

int
vscp_link_callback_statistics(vscp_link_ctx_t *pctx, vscp_statistics_t *pStatistics)
{
  if ((NULL == pctx) || (NULL == pStatistics)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  memcpy(pStatistics, &pctx->statistics, sizeof(vscp_statistics_t));

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_info
//

int
vscp_link_callback_info(vscp_link_ctx_t *pctx, vscp_status_t *pstatus)
{
  if ((NULL == pctx) || (NULL == pstatus)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  memcpy(pstatus, &pctx->status, sizeof(vscp_status_t));

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_rcvloop
//

int
vscp_link_callback_rcvloop(vscp_link_ctx_t *pctx, vscp_event_t **pev)
{
  // BaseType_t rv;
  // twai_message_t msg = {};

  // Check pointer
  if ((NULL == pctx) || (NULL == pev)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  // Every second output '+OK\r\n' in rcvloop mode
  if ((HAL_GetTick() - pctx->last_rcvloop_time) > 1000000) {
    pctx->last_rcvloop_time = HAL_GetTick();
    return VSCP_ERROR_TIMEOUT;
  }

  return vscp_link_callback_retr(pctx, pev);
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_wcyd
//

int
vscp_link_callback_wcyd(vscp_link_ctx_t *pctx, uint64_t *pwcyd)
{
  // Check pointers
  if ((NULL == pctx) || (NULL == pwcyd)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  // TODO
  *pwcyd = VSCP_SERVER_CAPABILITY_TCPIP | VSCP_SERVER_CAPABILITY_DECISION_MATRIX | VSCP_SERVER_CAPABILITY_IP4 |
           /*VSCP_SERVER_CAPABILITY_SSL |*/
           VSCP_SERVER_CAPABILITY_TWO_CONNECTIONS;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_shutdown
//

int
vscp_link_callback_shutdown(vscp_link_ctx_t *pctx)
{
  // Check pointers
  if (NULL == pctx) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  return VSCP_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_restart
//

int
vscp_link_callback_restart(vscp_link_ctx_t *pctx)
{
  // Check pointers
  if (NULL == pctx) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  NVIC_SystemReset();

  return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_link_callback_disconnect
//

int
vscp_link_callback_disconnect(vscp_link_ctx_t *pctx)
{
  if (NULL == pctx) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  // Restart ethernet module (will be ready for new connections again)
  wiznet_ip20_enter_command_mode();
  wiznet_ip20_restart();
  resetLinkContextDefaults(pctx);

  return VSCP_ERROR_SUCCESS;
}


