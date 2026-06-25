/*
  projdefs.h

  This file contains project definitions for the VSCP TCP/IP link protocol code.
*/

#ifndef _VSCP_PROJDEFS_H_
#define _VSCP_PROJDEFS_H_

// Debug logging macro. This can be used to print debug messages to the console.
// It is defined as a no-op by default, but it can be redefined to use printf or another logging mechanism if needed.
// #define LOGSTR(...) ((void)0)
#define LOGSTR(...) printf(__VA_ARGS__)

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

// Operation mode

#ifdef BLINKY_MODE_TCP_SERVER
#define WIZ_IP20_OPERATION_MODE "OP1\r\n" // OP1\r\n TCP server mode
#elif defined(BLINKY_MODE_TCP_CLIENT)
#define WIZ_IP20_OPERATION_MODE "OP0\r\n" // OP0\r\n TCP client mode
#elif defined(BLINKY_MODE_UDP)
#define WIZ_IP20_OPERATION_MODE "OP3\r\n" // OP3\r\n UDP mode
#elif defined(BLINKY_MODE_MULTICAST)
#define WIZ_IP20_OPERATION_MODE "OP3\r\n" // OP3\r\n Multicast mode
#elif defined(BLINKY_MODE_MQTT)
#define WIZ_IP20_OPERATION_MODE "OP5\r\n" // OP5\r\n MQTT mode
#else
#error "You must define WIZ_IP20_OPERATION_MODE based on the selected mode"
#endif

// Remote host (client) settings

#if defined(BLINKY_MODE_TCP_SERVER) || defined(BLINKY_MODE_TCP_CLIENT)
#define WIZ_IP20_REMOTE_HOST_IP   "RI192.168.1.7\r\n" // RI192.168.1.7\r\n
#define WIZ_IP20_REMOTE_HOST_PORT "RP9598\r\n"        // RP9598\r\n
#elif defined(BLINKY_MODE_UDP)
#define WIZ_IP20_REMOTE_HOST_IP   "RI255.255.255.255\r\n"
#define WIZ_IP20_REMOTE_HOST_PORT "RP33333\r\n" // RP33333\r\n
#elif defined(BLINKY_MODE_MULTICAST)
#define WIZ_IP20_REMOTE_HOST_IP   "RI224.0.23.158\r\n" // VSCP multicast address
#define WIZ_IP20_REMOTE_HOST_PORT "RP44444\r\n"
#elif defined(BLINKY_MODE_MQTT)
#define WIZ_IP20_REMOTE_HOST_IP   "RI192.168.1.7\r\n" // RI192.168.1.7\r\n
#define WIZ_IP20_REMOTE_HOST_PORT "RP1883\r\n"        // RP1883\r\n
#endif

#define WIZ_IP20_LINK_PACKING_TIME "PT0\r\n" // PT0\r\n data packing time in milliseconds
#define WIZ_IP20_LINK_PACKING_SIZE "PS0\r\n" // PS0\r\n data packing size in bytes (0 not used)
#define WIZ_IP20_LINK_PACKING_CHAR                                                                                     \
  "PD0a\r\n" // PD0a\r\n data packing char (0 not used, 0x0a = newline) The designated character is not included in
             // data.
#define WIZ_IP20_LINK_INACTIVITY                "IT0\r\n"    // IT0\r\n inactivity time in seconds (0 not used)
#define WIZ_IP20_LINK_RETRY_CNT                 "TR0\r\n"    // TR0\r\n retry count (0 not used)
#define WIZ_IP20_LINK_KEEP_ALIVE                "KA0\r\n"    // KA0\r\n keep alive (0 not used, 1 = enabled)
#define WIZ_IP20_LINK_KEEP_ALIVE_INTERVAL       "KI1000\r\n" // KI1000\r\n keep alive interval in milliseconds
#define WIZ_IP20_LINK_KEEP_ALIVE_RETRY_INTERVAL "KE5000\r\n" // KE5000\r\n keep alive retry interval in milliseconds

#define WIZ_IP20_LINK_SSL_CLOSE "SO0" // SO0\r\n SSL close (0 not used, 1 = enabled)

// TCP client settings (not used here)
#define WIZ_IP20_LINK_CLIENT_RECONNECT "RI3000\r\n" // RI3000\r\n TCP client reconnect interval in milliseconds

/*
  Strings to send on connection and disconnection. These are used to identify the device
  when a new connection is made or when a connection is lost. If you change you must make
  sure that you do change the strings in the main loop as well.
*/
#define WIZ_IP20_LINK_CONNECT_STR "SD" BLINKY_CONNECT_STR "\r\n" // SD<CONNECT>\r\n string to send on new connection
#define WIZ_IP20_LINK_DISCONNECT_STR                                                                                   \
  "DD" BLINKY_DISCONNECT_STR "\r\n" // DD<DISCONNECT>\r\n string to send on disconnection
#define WIZ_IP20_LINK_ETH_CONNECT_STR                                                                                  \
  "WIZNET-IP20\r\n" // SE\r\n string to send on new Ethernet connection (not used here)

#define WIZ_IP20_TCPSRV_ENABLE_PASSWORD "CP0\r\n" // CP0\r\n password for TCP server (0 not used)
// #define WIZ_IP20_TCPSRV_SET_PASSWORD  "NPsecret\r\n" // NPsecret\r\n password for TCP server (max 8 characters)
// #define WIZ_IP20_SEARCH_ID_CODE  "SPvscp\r\n" // SPvscp\r\n search id code (max 8 characters)
#define WIZ_IP20_DEBUG_ENABLE "DG0\r\n" // DG0\r\n enable debug messages (0 not used, 1 = enabled)

// MQTT settings (not used here)
#ifdef BLINKY_DEMO_MODE_MQTT
#define WIZ_IP20_MQTT_USER       "QUvscp\r\n"     // QUmqtt_user\r\n MQTT username (max 128 characters)
#define WIZ_IP20_MQTT_PASSWORD   "QPsecret\r\n"   // QPmqtt_password\r\n MQTT password (max 128 characters)
#define WIZ_IP20_MQTT_CLIENT_ID  "WCwiz_ip20\r\n" // QCmqtt_client_id\r\n MQTT client ID (max 128 characters)
#define WIZ_IP20_MQTT_KEEP_ALIVE "QK0\r\n"        // QK0\r\n MQTT keep alive (0 not used, 1 = enabled)
#define WIZ_IP20_MQTT_QOS        "QQ0\r\n"        // QQ0\r\n MQTT QoS (0, 1, or 2)
#define WIZ_IP20_MQTT_PUB_TOPIC                                                                                        \
  "PUvscp/{{guid}}/{{class}}/{{type}}\r\n"                   // PUtopic\r\n MQTT publish topic (max 128 characters)
#define WIZ_IP20_MQTT_SUB_TOPIC0 "U0vscp/{{guid}}/input\r\n" // U0topic\r\n MQTT subscribe topic 1 (max 128 characters)
#define WIZ_IP20_MQTT_SUB_TOPIC1 "U1mqtt/topic1\r\n"         // U1topic\r\n MQTT subscribe topic 2 (max 128 characters)
#define WIZ_IP20_MQTT_SUB_TOPIC2 "U2mqtt/topic2\r\n"         // U2topic\r\n MQTT subscribe topic 3 (max 128 characters)
#endif

/**
  ----------------------------------------------------------------------------
                        VSCP firmware helper defines
  ----------------------------------------------------------------------------
  Defines for firmware helper. These are used to link in support for
  features in the firmware helper. If you do not need support for a feature,
  you can save some memory by not linking it in.
*/

// We link in binary support from firmware helper.
#define VSCP_FWHLP_BINARY_FRAME_SUPPORT

// Crypto support is needed for the TCP/IP link protocol
#define VSCP_FWHLP_CRYPTO_SUPPORT

// SSL support is needed for crypto support
// #define VSCP_FWHLP_CRYPTO_USE_OPENSSL

// ON ESP32 we use the built in crypto support
// #define VSCP_FWHLP_CRYPTO_USE_PSA_CRYPTO

// Enable JSON support in firmware helper.
#define VSCP_FWHLP_JSON_SUPPORT

// Enable XML support in firmware helper.
#define VSCP_FWHLP_XML_SUPPORT

/**
  ----------------------------------------------------------------------------
                       VSCP Link Protocol defines
  ----------------------------------------------------------------------------
*/

/*!
  Define to show custom help. The callback is called so you can respond
  with your custom help text.  This can be used to save memory if you work
  on a memory constraint environment.

  If not defined, standard help is shown.
*/
// #define VSCP_LINK_CUSTOM_HELP_TEXT

/**
  ----------------------------------------------------------------------------
                       VSCP Firmware level II defines
  ----------------------------------------------------------------------------
  Defines for firmware level II
*/

/*!
  Name of device for level II capabilities announcement event.
  Max 32 characters.
*/
#define THIS_FIRMWARE_DEVICE_NAME "VSCP blinky tcp/ip link demo"

/**
 * Firmware version
 */
#define THIS_FIRMWARE_MAJOR_VERSION   (0u)
#define THIS_FIRMWARE_MINOR_VERSION   (0u)
#define THIS_FIRMWARE_RELEASE_VERSION (1u)
/* THIS_FIRMWARE_BUILD_VERSION is auto-generated and provided by build_number.h */
#include "build_number.h"


/**
 * Enable logging
 */
#define THIS_FIRMWARE_ENABLE_LOGGING

/**
 * Enable error reporting
 */
#define THIS_FIRMWARE_ENABLE_ERROR_REPORTING

/**
 * @brief Uncomment to enable writing to write protected areas
 *
 * Writing manufacturer data and GUID
 */
#define THIS_FIRMWARE_ENABLE_WRITE_2PROTECTED_LOCATIONS

/**
 * @brief Send server probe
 *
 */
#define THIS_FIRMWARE_VSCP_DISCOVER_SERVER


/// Buffer size for incoming TCP/IP data (not used here)
#define THIS_FIRMWARE_TCPIP_LINK_MAX_BUFFER         2048u
#define THIS_FIRMWARE_TCPIP_LINK_ENABLE_RCVLOOP_CMD 1

#endif // _VSCP_PROJDEFS_H_