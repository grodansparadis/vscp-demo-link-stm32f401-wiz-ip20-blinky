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

// Status register bits (register 0:6)
#define BLINKY_STATUS_LED_ON 0x02u // Bit 1 set when LED is on, clear when LED is off
#define BLINKY_STATUS_BTN_ON 0x01u // Bit 0 set when button is pressed, clear when button is released

// Control register bits (register 0:7)
#define BLINKY_CTRL_ENABLE_LED 0x80u // Bit 7 set to enable LED blinking, clear to disable
#define BLINKY_CTRL_ENABLE_BTN 0x40u // Bit 6 set to enable button, clear to disable
#define BLINKY_CTRL_ENABLE_BTN_TURNON 0x08u  // Bit 3 set to enable button TURN-ON event, clear to disable
#define BLINKY_CTRL_ENABLE_BTN_START 0x04u // Bit 2 set to enable button START event, clear to disable
#define BLINKY_CTRL_ENABLE_BTN_STOP 0x02u    // Bit 1 set to enable button STOP event, clear to disable

/*
  ----------------------------------------------------------------------------
                                   Blinky
  ----------------------------------------------------------------------------
*/


/*!
  Select **one **of the following modes for the blinky demo. This will determine how the
  blinky demo is built and will operate. You can only select one mode at a time. If you select
  more than one mode, you will get a compile error. If you do not select any mode,
  you will get a compile error.
*/

// VSCP TCP/IP link protocol server mode. The device will listen for incoming TCP/IP
// connections and respond to them. This is the default mode for the blinky demo.
#define BLINKY_MODE_TCP_SERVER

// VSCP TCP/IP link protocol client mode. The device will connect to a TCP/IP
// server and send data to it.
// #define BLINKY_MODE_TCP_CLIENT

// VSCP UDP link protocol mode. The device will listen for
// incoming UDP packets and respond to them.
// #define BLINKY_MODE_UDP

// VSCP multicast link protocol mode.
// The device will listen for incoming multicast packets and respond to them.
// #define BLINKY_MODE_MULTICAST

// VSCP MQTT link protocol mode.
// The device will connect to an MQTT broker and publish/subscribe to topics.
// #define BLINKY_MODE_MQTT

// Uncomment to get debug printouts on USART2
// #define VSCP_ENABLE_BLINKY_DEBUG

// ----------------------------------------------------------------------

// Buffer
#define BLINKY_TCPIP_BUF_MAX_SIZE (2048u)

/**
 * VSCP TCP link protocol character buffer size
 */
#ifndef BLINKY_DATA_BUF_SIZE
#define BLINKY_DATA_BUF_SIZE 512
#endif

/**
 * Max number of events in the incoming fifo (to node)
 */
#define BLINKY_INCOMING_FIFO_SIZE 16

/**
 * Max number of events in each of the outgoing fifo (to client)
 */
#define BLINKY_OUTGOING_FIFO_SIZE 16

#define BLINKY_WELCOME_MSG                                                                                             \
  "Welcome to the VSCP Blinky demo\r\n"                                                                                \
  "STM32F401 + WIZnet IP20\r\n"                                                                                        \
  "Version: 0.0.1 - %d\r\n"                                                                                            \
  "Copyright (C) 2000-2026 Grodans Paradis AB\r\n"                                                                     \
  "https://www.grodansparadis.com\r\n"                                                                                 \
  "+OK\r\n"

// System defaults


#define BLINKY_DEFAULT_ENCRYPTION_LEVEL VSCP_ENCRYPTION_NONE // 0 = none, 1 = AES128, 2 = AES192, 3 = AES256
#define BLINKY_DEFAULT_MODULE_ZONE      1                    // VSCP zone for module
#define BLINKY_DEFAULT_MODULE_SUBZONE   2                    // VSCP subzone for module

#define BLINKY_DEFAULT_VSCP_LINK_PORT     VSCP_LINK_PORT // defined in board-config.h
#define BLINKY_DEFAULT_VSCP_LINK_USER     "vscp"
#define BLINKY_DEFAULT_VSCP_LINK_PASSWORD "secret"

/**
 * Maximum number of simultaneous TCP/IP connections
 * This is the maximum simultaneous number
 * of connections to the server
 */
#define BLINKY_MAX_TCP_CONNECTIONS 1



#endif