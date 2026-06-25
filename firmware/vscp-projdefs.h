/*
  projdefs.h

  This file contains project definitions for the 
  VSCP TCP/IP link protocol code
  and the VSCP firmware level II code. It is used to configure
  the firmware helper and link protocol code for this specific project.
  It is included by the firmware helper and link protocol code.
  ******************************************************************************
*/

#ifndef _VSCP_PROJDEFS_H_
#define _VSCP_PROJDEFS_H_

// Debug logging macro. This can be used to print debug messages to the console.
// It is defined as a no-op by default, but it can be redefined to use printf or another logging mechanism if needed.
// #define LOGSTR(...) ((void)0)
#define LOGSTR(...) printf(__VA_ARGS__)


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