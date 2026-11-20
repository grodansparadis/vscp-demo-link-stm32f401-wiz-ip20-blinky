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

 /*
  ----------------------------------------------------------------------------
                          WIZnet IP20 settings
  ----------------------------------------------------------------------------

  WIZnet IP20 settings. These are used to set up the WIZnet IP20 module. You can
  change these settings to match your network configuration. Make sure to set the
  MAC address to something unique on your network.

  https://docs.wiznet.io/Product/Chip/MCU/Pre-programmed-MCU/W55RP20-S2E/command-manual-en#un

  Serial port settings are derived from the hardware configuration in board-config.h,
  which must match the CubeMX USART1 settings in stm32f401.ioc.
*/

#include "board-config.h"

/*
  This is the string that the system looks for to identify a new connection. It should be set to a value that
  is not likely to be sent by the client.
*/
#define BLINKY_CONNECT_STR "<CONNECT>"

/*
  This is the string that the system looks for to identify a client disconnecting. It should be set to a value that
  is not likely to be sent by the client.
*/
#define BLINKY_DISCONNECT_STR "<DISCONNECT>"



// Helper macros for stringification (used to build the WIZ_IP20_PORT command string)
#ifndef _BOARD_STR
#define _BOARD_STR(x)  #x
#define _BOARD_XSTR(x) _BOARD_STR(x)
#endif

// Serial port settings for WIZnet IP20 module - derived from board-config.h / USART1 configuration

// WIZnet baud rate codes: 0=300 1=600 2=1200 3=1800 4=2400 5=4800 6=9600 7=14400
//                         8=19200 9=28800 10=38400 11=57600 12=115200 13=230400
#if USART1_BAUDRATE == 300
#define WIZ_IP20_SERIAL_BAUDRATE "BR0\r\n"
#elif USART1_BAUDRATE == 600
#define WIZ_IP20_SERIAL_BAUDRATE "BR1\r\n"
#elif USART1_BAUDRATE == 1200
#define WIZ_IP20_SERIAL_BAUDRATE "BR2\r\n"
#elif USART1_BAUDRATE == 1800
#define WIZ_IP20_SERIAL_BAUDRATE "BR3\r\n"
#elif USART1_BAUDRATE == 2400
#define WIZ_IP20_SERIAL_BAUDRATE "BR4\r\n"
#elif USART1_BAUDRATE == 4800
#define WIZ_IP20_SERIAL_BAUDRATE "BR5\r\n"
#elif USART1_BAUDRATE == 9600
#define WIZ_IP20_SERIAL_BAUDRATE "BR6\r\n"
#elif USART1_BAUDRATE == 14400
#define WIZ_IP20_SERIAL_BAUDRATE "BR7\r\n"
#elif USART1_BAUDRATE == 19200
#define WIZ_IP20_SERIAL_BAUDRATE "BR8\r\n"
#elif USART1_BAUDRATE == 28800
#define WIZ_IP20_SERIAL_BAUDRATE "BR9\r\n"
#elif USART1_BAUDRATE == 38400
#define WIZ_IP20_SERIAL_BAUDRATE "BR10\r\n"
#elif USART1_BAUDRATE == 57600
#define WIZ_IP20_SERIAL_BAUDRATE "BR11\r\n"
#elif USART1_BAUDRATE == 115200
#define WIZ_IP20_SERIAL_BAUDRATE "BR12\r\n"
#elif USART1_BAUDRATE == 230400
#define WIZ_IP20_SERIAL_BAUDRATE "BR13\r\n"
#else
#error "Unsupported USART1_BAUDRATE. Update USART1_BAUDRATE in board-config.h."
#endif

// WIZnet data bits: DB0=7 bits, DB1=8 bits
#if USART1_DATABITS == 7
#define WIZ_IP20_SERIAL_DATABITS "DB0\r\n"
#elif USART1_DATABITS == 8
#define WIZ_IP20_SERIAL_DATABITS "DB1\r\n"
#else
#error "Unsupported USART1_DATABITS. Must be 7 or 8."
#endif

// WIZnet stop bits: SB0=1 stop bit, SB1=2 stop bits
#if USART1_STOPBITS == 1
#define WIZ_IP20_SERIAL_STOPBITS "SB0\r\n"
#elif USART1_STOPBITS == 2
#define WIZ_IP20_SERIAL_STOPBITS "SB1\r\n"
#else
#error "Unsupported USART1_STOPBITS. Must be 1 or 2."
#endif

// WIZnet parity: PR0=none, PR1=odd, PR2=even
#if USART1_PARITY == 0
#define WIZ_IP20_SERIAL_PARITY "PR0\r\n"
#elif USART1_PARITY == 1
#define WIZ_IP20_SERIAL_PARITY "PR1\r\n"
#elif USART1_PARITY == 2
#define WIZ_IP20_SERIAL_PARITY "PR2\r\n"
#else
#error "Unsupported USART1_PARITY. Must be 0 (none), 1 (odd), or 2 (even)."
#endif

// WIZnet flow control: FL0=none, FL1=XON/XOFF, FL2=RTS/CTS, FL3=RTS on TX, FL4=RTS on TX(invert)
#if USART1_FLOWCTRL == 0
#define WIZ_IP20_SERIAL_FLOWCTRL "FL0\r\n"
#elif USART1_FLOWCTRL == 1
#define WIZ_IP20_SERIAL_FLOWCTRL "FL1\r\n"
#elif USART1_FLOWCTRL == 2
#define WIZ_IP20_SERIAL_FLOWCTRL "FL2\r\n"
#elif USART1_FLOWCTRL == 3
#define WIZ_IP20_SERIAL_FLOWCTRL "FL3\r\n"
#elif USART1_FLOWCTRL == 4
#define WIZ_IP20_SERIAL_FLOWCTRL "FL4\r\n"
#else
#error "Unsupported USART1_FLOWCTRL. Must be 0-4."
#endif

#define WIZ_IP20_SERIAL_ECHO "EC1\r\n" // EC0\r\n 0 = not used, 1 = used

#define WIZ_IP20_IP                   "LI192.168.1.88\r\n"                    // LI192.168.1.88\r\n
#define WIZ_IP20_NETMASK              "SM255.255.255.0\r\n"                   // SM255.255.255.0\r\n
#define WIZ_IP20_GATEWAY              "GW192.168.1.1\r\n"                     // GW192.168.1.1\r\n
#define WIZ_IP20_DNS                  "DS8.8.8.8\r\n"                         // DS8.8.8.8\r\n
#define WIZ_IP20_IP_ALLOCATION_METHOD "IM0\r\n"                               // IM0 = static, IM1 = DHCP, IM2 = PPPoE
#define WIZ_IP20_PORT                 "LP" _BOARD_XSTR(VSCP_LINK_PORT) "\r\n" // derived from VSCP_LINK_PORT in board-config.h

// ---------------------------------------------------------------------------
// Transport settings (see BLINKY_MODE_* in blinky.h). Commands are from the
// W55RP20-S2E command manual; the module is configured at every start.
// ---------------------------------------------------------------------------

// Remote host: the link server for TCP client, the UDP peer, the MQTT broker.
// Override with -DBLINKY_REMOTE_HOST=\"192.168.1.9\" etc.
#ifndef BLINKY_REMOTE_HOST
#define BLINKY_REMOTE_HOST "192.168.1.7"
#endif

#ifdef BLINKY_MODE_TCP_SERVER
#define WIZ_IP20_OPERATION_MODE "OP1\r\n" // TCP server
#ifndef BLINKY_LOCAL_PORT
#define BLINKY_LOCAL_PORT VSCP_LINK_PORT
#endif
#elif defined(BLINKY_MODE_TCP_CLIENT)
#define WIZ_IP20_OPERATION_MODE "OP0\r\n" // TCP client
#ifndef BLINKY_REMOTE_PORT
#define BLINKY_REMOTE_PORT VSCP_LINK_PORT
#endif
#elif defined(BLINKY_MODE_UDP)
#define WIZ_IP20_OPERATION_MODE "OP3\r\n" // UDP
#ifndef BLINKY_LOCAL_PORT
#define BLINKY_LOCAL_PORT 33333 // VSCP UDP port
#endif
#ifndef BLINKY_REMOTE_PORT
#define BLINKY_REMOTE_PORT 33333
#endif
#elif defined(BLINKY_MODE_MULTICAST)
#define WIZ_IP20_OPERATION_MODE "OP3\r\n" // UDP, multicast remote address
#undef BLINKY_REMOTE_HOST
#define BLINKY_REMOTE_HOST "224.0.23.158" // VSCP multicast group
#ifndef BLINKY_LOCAL_PORT
#define BLINKY_LOCAL_PORT 44444 // VSCP multicast port
#endif
#ifndef BLINKY_REMOTE_PORT
#define BLINKY_REMOTE_PORT 44444
#endif
#elif defined(BLINKY_MODE_MQTT)
#define WIZ_IP20_OPERATION_MODE "OP5\r\n" // MQTT client
#ifndef BLINKY_REMOTE_PORT
#define BLINKY_REMOTE_PORT 1883 // MQTT broker port
#endif
#endif

#ifndef BLINKY_LOCAL_PORT
#define BLINKY_LOCAL_PORT 0 // Any local port (client modes)
#endif

#define WIZ_IP20_LINK_CLIENT_RECONNECT "RI3000\r\n" // TCP client/MQTT reconnect interval in ms

// Packing of serial data into network data:
//   TCP (link protocol) and MQTT: lines, packed at the newline (PD0a).
//   UDP/multicast: one datagram per frame: binary frames are written in one
//   piece and sent when the serial line has been idle for 3 ms (PT3).
#ifdef BLINKY_TRANSPORT_LINK
#define WIZ_IP20_LINK_PACKING_TIME "PT0\r\n"
#define WIZ_IP20_LINK_PACKING_CHAR "PD0a\r\n" // Link protocol lines (as before)
#elif defined(BLINKY_MODE_MQTT)
#define WIZ_IP20_LINK_PACKING_TIME "PT0\r\n"
#define WIZ_IP20_LINK_PACKING_CHAR "PD0a\r\n" // One MQTT message per line
#else
#define WIZ_IP20_LINK_PACKING_TIME "PT3\r\n"
#define WIZ_IP20_LINK_PACKING_CHAR "PD00\r\n" // Binary frames: no delimiter character
#endif
#define WIZ_IP20_LINK_PACKING_SIZE "PS0\r\n" // No size packing

#define WIZ_IP20_LINK_INACTIVITY                "IT0\r\n"    // inactivity time in seconds (0 not used)
#define WIZ_IP20_LINK_KEEP_ALIVE                "KA0\r\n"    // keep alive (0 not used, 1 = enabled)
#define WIZ_IP20_LINK_KEEP_ALIVE_INTERVAL       "KI1000\r\n" // keep alive interval in milliseconds
#define WIZ_IP20_LINK_KEEP_ALIVE_RETRY_INTERVAL "KE5000\r\n" // keep alive retry interval in milliseconds
#define WIZ_IP20_LINK_SSL_CLOSE                 "SO0\r\n"    // SSL close (0 not used)

/*
  Strings the module writes to the serial port when a TCP connection opens
  and closes. The main loop looks for them in the TCP modes; in the other
  modes they are empty (not used) so they do not mix with the data.
*/
#ifdef BLINKY_TRANSPORT_LINK
#define WIZ_IP20_LINK_CONNECT_STR    "SD" BLINKY_CONNECT_STR "\r\n"
#define WIZ_IP20_LINK_DISCONNECT_STR "DD" BLINKY_DISCONNECT_STR "\r\n"
#else
#define WIZ_IP20_LINK_CONNECT_STR    "SD\r\n"
#define WIZ_IP20_LINK_DISCONNECT_STR "DD\r\n"
#endif
#define WIZ_IP20_LINK_ETH_CONNECT_STR "SE\r\n" // Nothing is sent over Ethernet on connect

#define WIZ_IP20_TCPSRV_ENABLE_PASSWORD "CP0\r\n" // no password for TCP server
#define WIZ_IP20_DEBUG_ENABLE           "DG0\r\n" // no debug messages

// MQTT (BLINKY_MODE_MQTT). The topics are vscp/<guid>/out and vscp/<guid>/in.
#ifndef BLINKY_MQTT_USER
#define BLINKY_MQTT_USER "vscp"
#endif
#ifndef BLINKY_MQTT_PASSWORD
#define BLINKY_MQTT_PASSWORD "secret"
#endif
#define BLINKY_MQTT_KEEP_ALIVE 60 // seconds
#define BLINKY_MQTT_QOS        0

/*!
 * @brief Print help information for the W55RP20-S2E module.
 */

void
wiznet_ip20_printHelp();

/*!
 * @brief  Restart the WIZnet IP20 module to apply new settings.
 *
 * After saving the configuration, we need to restart the WIZnet IP20 module for the new settings to take effect. This
 * function sends the appropriate AT command to trigger a restart of the module. The exact command may depend on the
 * module's firmware version, so you should refer to the WIZnet IP20 documentation for the correct command to use for
 * restarting the module.
 *
 * Note: This function assumes that you are already in command mode before calling it.
 *
 * @retval None
 */

void
wiznet_ip20_restart(void);

/*!
 * @brief  Enter command mode for the WIZnet IP20 module.
 *
 * This function sends the appropriate AT command to the WIZnet IP20 module to enter command mode. In command mode, you
 * can configure various settings of the module. The exact command may depend on the module's firmware version, so you
 * should refer to the WIZnet IP20 documentation for the correct command to use for entering command mode.
 *
 * @retval int  Returns 0 on success, or a negative value on error.
 */

int
wiznet_ip20_enter_command_mode(void);

/*!
 * @brief  Save WIZnet IP20 configuration to non-volatile memory.
 *
 * After configuring the WIZnet IP20 module with the desired settings using AT commands, we need to save the
 * configuration to non-volatile memory so that it persists across power cycles. This function sends the appropriate AT
 * command to trigger the save operation on the WIZnet IP20 module. The exact command may depend on the module's
 * firmware version, so you should refer to the WIZnet IP20 documentation for the correct command to use for saving the
 * configuration.
 *
 * Note: This function assumes that you are already in command mode before calling it.
 *
 * @retval None
 */

void
wiznet_ip20_save(void);

/*!
 * @brief  Initialize the WIZnet IP20 module with the desired network settings and link protocol configuration.
 *
 * This function sends a series of AT commands to the WIZnet IP20 module over UART1 to configure its network settings
 * (IP address, netmask, gateway, DNS) and link protocol parameters (port, operation mode, connect/disconnect strings,
 * etc.) according to the defined constants. It uses the sendCommand() helper function to send each command and wait for
 * a response. The exact commands sent depend on the specific configuration you want for your application, and you
 * should refer to the WIZnet IP20 documentation for details on the available AT commands and their syntax.
 *
 * Note: This function assumes that you have already entered command mode using goCommandMode() before calling it.
 *
 * @retval VSCP_ERROR_SUCCESS if initialization was successful, VSCP error code on error.
 */

int
wiznet_ip20_init(const uint8_t *guid);

/*!
 * @brief  Send a command to the WIZnet IP20 module and receive the response.
 *
 * This function sends a command string to the WIZnet IP20 module over UART and waits for a response. The response is
 * stored in the provided buffer. The function will wait for a specified timeout period for the response to be
 * received.
 *
 * @param cmd Pointer to a null-terminated string containing the command to send.
 * @param response_buf Pointer to a buffer where the response will be stored.
 * @param response_buf_size Size of the response buffer in bytes.
 * @param timeout_ms Timeout in milliseconds to wait for a response.
 * @retval int Returns 0 on success, or a negative value on error (e.g., timeout, buffer too small).
 */

int
wiznet_ip20_send_command(const char *cmd, char *response_buf, size_t response_buf_size, uint16_t timeout_ms);

/*!
 * @brief  Show the current settings of the WIZnet IP20 module.
 *
 * This function retrieves and displays the current configuration settings of the WIZnet IP20 module. It sends the
 * appropriate AT commands to query the module's settings and prints the results.
 *
 * @retval int Returns 0 on success, or a negative value on error.
 */

int
wiznet_ip20_show_settings(void);

/*!
  * @brief  Get UART response with timeout (blocking, used during AT init only).
  * @param  buf: Buffer to store the received data.
  * @param  buf_size: Size of the buffer.
  * @param  timeout_ms: Timeout in milliseconds.
  * @retval Number of bytes received, or 0 on timeout.
  */

size_t
wiznet_ip20_getResponse(char *buf, size_t buf_size, uint32_t timeout_ms);