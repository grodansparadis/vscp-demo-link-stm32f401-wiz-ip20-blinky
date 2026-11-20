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
  //uint8_t guid[16];    // GUID for node ()
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

// User registers ordinals
#define BLINKY_REG_DEVICE_ZONE               0 // Zone for the device [P]
#define BLINKY_REG_DEVICE_SUBZONE            1 // Subzone for the device [P]
#define BLINKY_REG_DEVICE_STATUS             2 // Status of the device [P]
#define BLINKY_REG_DEVICE_CONTROL            3 // Control register for the device [P]
#define BLINKY_REG_DEVICE_BLINK_INTERVAL_MSB 4 // Most significant byte of the blink interval [P]
#define BLINKY_REG_DEVICE_BLINK_INTERVAL_LSB 5 // Least significant byte of the blink interval [P]
#define BLINKY_REG_DEVICE_COUNTER_0          6 // Counter 0 
#define BLINKY_REG_DEVICE_COUNTER_1          7 // Counter 1 
#define BLINKY_REG_DEVICE_COUNTER_2          8 // Counter 2 
#define BLINKY_REG_DEVICE_COUNTER_3          9 // Counter 3 
#define BLINKY_REG_DEVICE_BUTTON_BYTE0       10 // Button byte 0 [P]
#define BLINKY_REG_DEVICE_BUTTON_ZONE        11 // Button zone [P]
#define BLINKY_REG_DEVICE_BUTTON_SUBZONE     12 // Button subzone [P]

// Status register bits (register 0:6)
#define BLINKY_STATUS_LED_ON 0x02u // Bit 1 set when LED is on, clear when LED is off
#define BLINKY_STATUS_BTN_ON 0x01u // Bit 0 set when button is pressed, clear when button is released

// Control register bits (register 0:7)
#define BLINKY_CTRL_ENABLE_LED        0x80u // Bit 7 set to enable LED blinking, clear to disable
#define BLINKY_CTRL_ENABLE_BTN        0x40u // Bit 6 set to enable button, clear to disable
#define BLINKY_CTRL_ENABLE_BTN_TURNON 0x08u // Bit 3 set to enable button TURN-ON event, clear to disable
#define BLINKY_CTRL_ENABLE_BTN_START  0x04u // Bit 2 set to enable button START event, clear to disable
#define BLINKY_CTRL_ENABLE_BTN_STOP   0x02u // Bit 1 set to enable button STOP event, clear to disable
#define BLINKY_CTRL_ENABLE_MS_COUNTER 0x01u // Bit 0 set to enable millisecond counter, clear to disable

#define BLINKY_DEFAULT_REG_ZONE            0u    // Default zone for the device
#define BLINKY_DEFAULT_REG_SUBZONE         0u    // Default subzone for the device
#define BLINKY_DEFAULT_REG_BUTTON_ZONE     0u    // Default button zone for the device
#define BLINKY_DEFAULT_REG_BUTTON_SUBZONE  0u    // Default button subzone for the device
#define BLINKY_DEFAULT_REG_BUTTON_OPT_BYTE 0u    // Default button option byte for the device
#define BLINKY_DEFAULT_REG_BLINK_INTERVAL  500u  // Default blink interval in milliseconds (MDF: 0x01F4)
// Default control register as in the MDF: 0b11001110 (the millisecond counter is off)
#define BLINKY_DEFAULT_REG_CONTROL                                                                                     \
  (BLINKY_CTRL_ENABLE_LED | BLINKY_CTRL_ENABLE_BTN | BLINKY_CTRL_ENABLE_BTN_TURNON | BLINKY_CTRL_ENABLE_BTN_START |    \
   BLINKY_CTRL_ENABLE_BTN_STOP)
#define BLINKY_DEFAULT_REG_NICKNAME 0x10u // Default nickname for the device

// Actions

#define BLINKY_ACTION_NOOP                 0 // No operation
#define BLINKY_ACTION_CTRL_LED_STATE       1 // Control LED state (arg=0=OFF, arg=1=ON)
#define BLINKY_ACTION_CTRL_LED_ON          2 // Turn LED on
#define BLINKY_ACTION_CTRL_LED_OFF         3 // Turn LED off
#define BLINKY_ACTION_CTRL_LED_TOGGLE      4 // Toggle LED state
#define BLINKY_ACTION_CTRL_LED_START_BLINK 5 // Start LED blinking
#define BLINKY_ACTION_CTRL_LED_STOP_BLINK  6 // Stop LED blinking
#define BLINKY_ACTION_CLR_COUNTER          7 // Clear millisecond counter

// Action parameters

// Control LED state (arg=0=OFF, arg=1=ON)
#define BLINKY_ARG_CTRL_LED_OFF    0
#define BLINKY_ARG_CTRL_LED_ON     1
#define BLINKY_ARG_CTRL_LED_TOGGLE 2

// Decision matrix: four Level I rows of eight bytes at page 0, offset 0x60 (MDF)
#define BLINKY_REG_DM_START 0x60
#define BLINKY_DM_ROWS      4
#define BLINKY_DM_ROW_SIZE  8

// Alarm status bit set when the millisecond counter wraps (MDF alarm bit 7)
#define BLINKY_ALARM_COUNTER_LIMIT 0x80u

// Changed configuration is saved this long after the last change, so a
// burst of register writes is saved as one flash record
#define BLINKY_SAVE_DELAY_MS 1000u

// Button events from the interrupt handler, handled in blinky_work()
#define BLINKY_BUTTON_PRESSED  0x01u
#define BLINKY_BUTTON_RELEASED 0x02u

extern volatile uint8_t g_button_events;   // BLINKY_BUTTON_PRESSED/RELEASED, set by the EXTI handler
extern volatile uint8_t g_counter_wrapped; // Set by the 1 ms timer when the counter wraps to zero

/*
  Blinky functionality (callbacks-frm2.c). pfw is the Level II firmware
  context; its user data is the link protocol context.
*/

// Set up the LED and blinking from the registers after they are loaded
void
blinky_init(vscp_frmw2_firmware_context_t *pfw);

// Call from the main loop: button events, blinking, counter alarm, saving
void
blinky_work(vscp_frmw2_firmware_context_t *pfw);

// Turn the LED on/off. With report set, CLASS1.INFORMATION ON/OFF is sent if it changed.
void
blinky_set_led(vscp_frmw2_firmware_context_t *pfw, int on, int report);

// Queue an event for the connected client (dropped when no client is connected)
int
blinky_send_event(vscp_frmw2_firmware_context_t *pfw,
                  uint16_t vscp_class,
                  uint16_t vscp_type,
                  const uint8_t *pdata,
                  uint8_t size);

// Save the registers to flash a little later (BLINKY_SAVE_DELAY_MS)
void
blinky_request_save(void);

// Save changed registers now (when a session ends, before slow work)
void
blinky_save_now(void);

/*
  ----------------------------------------------------------------------------
                                   Blinky
  ----------------------------------------------------------------------------
*/

/*!
  Transport (one of them). Select it with the CMake option BLINKY_MODE:

    cmake -B build -DBLINKY_MODE=TCP_SERVER   (default)
                   -DBLINKY_MODE=TCP_CLIENT
                   -DBLINKY_MODE=UDP
                   -DBLINKY_MODE=MULTICAST
                   -DBLINKY_MODE=MQTT

  BLINKY_MODE_TCP_SERVER  VSCP link protocol (TCP/IP), the device is the server (port 9598).
  BLINKY_MODE_TCP_CLIENT  VSCP link protocol, the device connects to BLINKY_REMOTE_HOST:BLINKY_REMOTE_PORT
                          and is the link protocol server on that connection.
  BLINKY_MODE_UDP         VSCP frames (version 0, not encrypted) over UDP: sent to
                          BLINKY_REMOTE_HOST:BLINKY_REMOTE_PORT, received on BLINKY_LOCAL_PORT.
  BLINKY_MODE_MULTICAST   VSCP frames to/from the VSCP multicast group 224.0.23.158, port 44444.
  BLINKY_MODE_MQTT        VSCP JSON events, one per MQTT message, published on vscp/<guid>/out and
                          received on vscp/<guid>/in, broker BLINKY_REMOTE_HOST:BLINKY_REMOTE_PORT.
*/

#if !defined(BLINKY_MODE_TCP_SERVER) && !defined(BLINKY_MODE_TCP_CLIENT) && !defined(BLINKY_MODE_UDP) &&            \
  !defined(BLINKY_MODE_MULTICAST) && !defined(BLINKY_MODE_MQTT)
#define BLINKY_MODE_TCP_SERVER
#endif

#if (defined(BLINKY_MODE_TCP_SERVER) + defined(BLINKY_MODE_TCP_CLIENT) + defined(BLINKY_MODE_UDP) +                    \
     defined(BLINKY_MODE_MULTICAST) + defined(BLINKY_MODE_MQTT)) != 1
#error "Select exactly one BLINKY_MODE_..."
#endif

// The link protocol (text lines) is used in the TCP modes
#if defined(BLINKY_MODE_TCP_SERVER) || defined(BLINKY_MODE_TCP_CLIENT)
#define BLINKY_TRANSPORT_LINK 1
#endif

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