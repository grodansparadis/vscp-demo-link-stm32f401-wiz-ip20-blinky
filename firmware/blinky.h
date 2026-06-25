/*
  File: tcpsrv.c

  VSCP Wireless CAN4VSCP Gateway (VSCP-WCANG)

  This file is part of the VSCP (https://www.vscp.org)

  The MIT License (MIT)
  Copyright (C) 2022-2026 Ake Hedman, the VSCP project <info@vscp.org>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef __VSCPBLINK_H__
#define __VSCPBLINK_H__

#include "board-config.h"

#include <vscp.h>
#include <vscp-fifo.h>
#include <vscp-firmware-helper.h>
#include <vscp-link-protocol.h>
#include <vscp-firmware-level2.h>



typedef struct {

  // Module
  char nodeName[32];   // User name for node
  uint8_t nodeZone;    // VSCP zone for node
  uint8_t nodeSubzone; // VSCP subzone for node
  uint8_t guid[16];    // GUID for node (default: Constructed from MAC address)
  uint8_t encryptLvl;  // Encryption level for UDP messages (0 = none, 1 = AES128, 2 = AES192, 3 = AES256)
  uint8_t pmk[16];     // System security key for encryption (AES128)
  uint8_t pmkLen;      // For future use, Now always 16 (AES128)
  uint32_t bootCnt;    // Number of restarts (not editable)

  // VSCP link protocol
  uint16_t vscplinkPort;                             // VSCP link protocol port
  char vscplinkUser[VSCP_LINK_MAX_USER_NAME_LENGTH]; // VSCP link protocol user
  char vscplinkPw[VSCP_LINK_MAX_PASSWORD_LENGTH];    // VSCP link protocol password

} node_persistent_config_t;

// Message for connection when max number of clients is reached
#define MSG_MAX_CLIENTS "Max number of clients reached. Disconnecting.\r\n"

// This string can be  displayed when the user types "HELP" or "?" in the command
// line interface if the standard help text takes to much resources.
#define BLINKY_HELP "See https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_tcpiplink for help\r\n"

#endif