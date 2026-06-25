// FILE: wiznet-ip20.c

// This file holds routined for the wiznet ip20 module
//

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

#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include <stdint.h>
#include <string.h>
#include <blinky.h>
#include "wiznet-ip20.h"

extern char g_ipaddrstr[20];  // main.c
extern char g_macaddrstr[20]; // main.c

// Command help
void
wiznet_ip20_printHelp()
{
  printf("<<< W55RP20-S2E AT Help >>>\r\n");
  printf("Enter command mode: +++ (guard time >= 500ms before/after)\r\n");
  printf("Exit command mode: EX\r\n");
  printf("Save settings: SV  | Reboot: RT  | Factory reset: FR\r\n");
  printf("\r\n");
  printf("[Device Info] (RO)\r\n");
  printf("MC  -> MAC address (ex: MC00:08:DC:00:00:01)\r\n");
  printf("VR  -> Firmware version (ex: VR1.0.0)\r\n");
  printf("MN  -> Product name (ex: MNWIZ5XXRSR-RP)\r\n");
  printf("ST  -> Status (BOOT/OPEN/CONNECT/UPGRADE/ATMODE)\r\n");
  printf("UN  -> UART interface str (ex: UNRS-232/TTL)\r\n");
  printf("UI  -> UART interface code (ex: UI0)\r\n");
  printf("\r\n");
  printf("[Network] (RW)\r\n");
  printf("OPx -> Mode: 0 TCP client, 1 TCP server, 2 mixed, 3 UDP, 4 SSL, 5 MQTT, 6 MQTTS\r\n");
  printf("IMx -> IP alloc: 0 static, 1 DHCP\r\n");
  printf("LIa.b.c.d -> Local IP (ex: LI192.168.11.2)\r\n");
  printf("SMa.b.c.d -> Subnet (ex: SM255.255.255.0)\r\n");
  printf("GWa.b.c.d -> Gateway (ex: GW192.168.11.1)\r\n");
  printf("DSa.b.c.d -> DNS (ex: DS8.8.8.8)\r\n");
  printf("LPn -> Local port (ex: LP5000)\r\n");
  printf("RHa.b.c.d / domain -> Remote host (ex: RH192.168.11.3)\r\n");
  printf("RPn -> Remote port (ex: RP5000)\r\n");
  printf("\r\n");
  printf("[UART] (RW)\r\n");
  printf("BRx -> Baud (12=115200, 13=230400)\r\n");
  printf("DBx -> Data bits (0=7bit, 1=8bit)\r\n");
  printf("PRx -> Parity (0=None, 1=Odd, 2=Even)\r\n");
  printf("SBx -> Stop bits (0=1bit, 1=2bit)\r\n");
  printf("FLx -> Flow (0=None, 1=XON/XOFF, 2=RTS/CTS)\r\n");
  printf("ECx -> Echo (0=Off, 1=On)\r\n");
  printf("\r\n");
  printf("[Packing] (RW)\r\n");
  printf("PTn -> Time delimiter ms (ex: PT1000)\r\n");
  printf("PSn -> Size delimiter bytes (ex: PS64)\r\n");
  printf("PDxx -> Char delimiter hex (ex: PD0D)\r\n");
  printf("\r\n");
  printf("[Options] (RW)\r\n");
  printf("ITn -> Inactivity sec (ex: IT30)\r\n");
  printf("RIn -> Reconnect interval ms (ex: RI3000)\r\n");
  printf("CPx -> Conn password enable (0/1)\r\n");
  printf("NPxxxx -> Conn password (max 8 chars)\r\n");
  printf("SPxxxx -> Search ID (max 8 chars)\r\n");
  printf("DGx -> Debug msg (0/1)\r\n");
  printf("KAx -> Keep-alive (0/1)\r\n");
  printf("KIn -> KA initial interval ms (ex: KI7000)\r\n");
  printf("KEn -> KA retry interval ms (ex: KE5000)\r\n");
  printf("SOn -> SSL recv timeout ms (ex: SO2000)\r\n");
  printf("\r\n");
  printf("[MQTT] (RW)\r\n");
  printf("QUuser QPpass QCid QK60 PUtopic\r\n");
  printf("U0sub U1sub U2sub QO0\r\n");
  printf("\r\n");
  printf("Type HELP or ? to show this list again.\r\n");
}

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
wiznet_ip20_restart(void)
{
  char buf[80];

  HAL_UART_Transmit(&huart1, (uint8_t *) "RT\r\n", 8, HAL_MAX_DELAY);
  if (0 == uart1_rx_getline(buf, sizeof(buf), 1000)) {
    LOGSTR("Response: %s\r\n", buf);
  }
}

/*!
 * @brief  Enter WIZnet IP20 command mode and wait for the "CMD" response.
 *
 * The WIZnet IP20 module uses an AT command interface for configuration. To send
 * AT commands, we first need to enter command mode by sending "+++" with a guard
 * time before and after. After sending "+++", we wait for the "CMD" response to
 * confirm that we are in command mode before proceeding with further configuration.
 *
 * @retval 0 if successfully entered command mode, -1 on timeout or error.
 */
int
wiznet_ip20_enter_command_mode(void)
{
  size_t rx_len;

  char buf[80];

  LOGSTR("WIZnet IP20 enter command mode:      \r\n");

  // Enter command mode: guard time, "+++", guard time
  HAL_Delay(500); // guard time before
  HAL_UART_Transmit(&huart1, (uint8_t *) "+++", 3, HAL_MAX_DELAY);
  HAL_Delay(500); // guard time after
  if (0 == uart1_rx_getline(buf, sizeof(buf), 1000)) {
    LOGSTR("Received response after +++: %s\r\n", buf);
    if (strstr(buf, "SEG:AT Mode") == 0) {
      LOGSTR("Entered command mode successfully %s\r\n", buf);
      return 0;
    }
  }

  LOGSTR("Failed to enter command mode (maybe already in command mode)\r\n");

  // If already in command mode this will give error but we get back to command mode anyway so we can ignore it
  HAL_UART_Transmit(&huart1, (uint8_t *) "\r\n", 2, HAL_MAX_DELAY);
  uart1_rx_getline(buf, sizeof(buf), 1000); // Clear any response from the "+++" command

  return 0;
}

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
wiznet_ip20_save(void)
{
  // For some strange reason two SV is needed to really save
  HAL_UART_Transmit(&huart1, (uint8_t *) "SV\r\nSV\r\n", 8, HAL_MAX_DELAY);
}

/*!
 * @brief  Send an AT command to the WIZnet IP20 module and wait for a response.
 *
 * This function sends a specified AT command string to the WIZnet IP20 module over UART1 and waits for a response. The
 * response is expected to be a complete line terminated by \r\n. The function uses the uart1_rx_getline() helper
 * function to read the response from the ring buffer with a specified timeout. If a response is received within the
 * timeout, it is stored in the provided response buffer and the function returns success. If no response is received
 * within the timeout, the function returns failure.
 *
 * Note: This function assumes that you are already in command mode before calling it.
 *
 * @param cmd                The AT command string to send (should include \r\n if required by the command).
 * @param response_buf       Buffer to store the received response (should be large enough to hold expected responses).
 * @param response_buf_size  Size of the response buffer in bytes.
 * @param timeout_ms         Timeout in milliseconds to wait for a response.
 * @retval int               Returns 1 on success (response received), or 0 on failure (timeout or error).
 */

int
wiznet_ip20_send_command(const char *cmd, char *response_buf, size_t response_buf_size, uint16_t timeout_ms)
{
  char buf[100];
  LOGSTR("WIZnet IP20 send command: %s\r\n", cmd);
  HAL_UART_Transmit(&huart1, (uint8_t *) cmd, strlen(cmd), HAL_MAX_DELAY);
  if (-1 == uart1_rx_getline(buf, sizeof(buf), timeout_ms)) {
    LOGSTR("Failed to send command: %s\r\n", cmd);
    return 0;
  }
  return 1;
}

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
wiznet_ip20_init(void)
{
  char buf[100];
  size_t rx_len;

  // Set IP address
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_IP, strlen(WIZ_IP20_IP), HAL_MAX_DELAY);

  // Set netmask
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_NETMASK, strlen(WIZ_IP20_NETMASK), HAL_MAX_DELAY);

  // Set gateway
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_GATEWAY, strlen(WIZ_IP20_GATEWAY), HAL_MAX_DELAY);

  // Set DNS server
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_DNS, strlen(WIZ_IP20_DNS), HAL_MAX_DELAY);

  // Set IP allocation mode
  HAL_UART_Transmit(&huart1,
                    (uint8_t *) WIZ_IP20_IP_ALLOCATION_METHOD,
                    strlen(WIZ_IP20_IP_ALLOCATION_METHOD),
                    HAL_MAX_DELAY);

  // Set port
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_PORT, strlen(WIZ_IP20_PORT), HAL_MAX_DELAY);

  // Set operation mode
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_OPERATION_MODE, strlen(WIZ_IP20_OPERATION_MODE), HAL_MAX_DELAY);

  // Set connect string
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_LINK_CONNECT_STR, strlen(WIZ_IP20_LINK_CONNECT_STR), HAL_MAX_DELAY);

  // Set disconnect string
  HAL_UART_Transmit(&huart1,
                    (uint8_t *) WIZ_IP20_LINK_DISCONNECT_STR,
                    strlen(WIZ_IP20_LINK_DISCONNECT_STR),
                    HAL_MAX_DELAY);

  // Set Ethernet connect string
  HAL_UART_Transmit(&huart1,
                    (uint8_t *) WIZ_IP20_LINK_ETH_CONNECT_STR,
                    strlen(WIZ_IP20_LINK_ETH_CONNECT_STR),
                    HAL_MAX_DELAY);

#if defined(BLINKY_MODE_TCP_CLIENT) || defined(BLINKY_MODE_MQTT)
  // Remote host
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_REMOTE_HOST_IP, strlen(WIZ_IP20_REMOTE_HOST_IP), HAL_MAX_DELAY);

  // Remote port
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_REMOTE_HOST_PORT, strlen(WIZ_IP20_REMOTE_HOST_PORT), HAL_MAX_DELAY);

  // TCP client reconnect interval
  HAL_UART_Transmit(&huart1,
                    (uint8_t *) WIZ_IP20_LINK_CLIENT_RECONNECT,
                    strlen(WIZ_IP20_LINK_CLIENT_RECONNECT),
                    HAL_MAX_DELAY);
#endif

  // Data packing time in milliseconds
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_LINK_PACKING_TIME, strlen(WIZ_IP20_LINK_PACKING_TIME), HAL_MAX_DELAY);

  // Data packing size in bytes (0 not used)
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_LINK_PACKING_SIZE, strlen(WIZ_IP20_LINK_PACKING_SIZE), HAL_MAX_DELAY);

  // Data packing char (0 not used, 0x0a = newline) The designated character is not included in data.
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_LINK_PACKING_CHAR, strlen(WIZ_IP20_LINK_PACKING_CHAR), HAL_MAX_DELAY);

  // Inactivity time in seconds (0 not used)
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_LINK_INACTIVITY, strlen(WIZ_IP20_LINK_INACTIVITY), HAL_MAX_DELAY);

  // Retry count (0 not used)
  // HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_LINK_RETRY_CNT, strlen(WIZ_IP20_LINK_RETRY_CNT), HAL_MAX_DELAY);

  // Keep alive (0 not used, 1 = enabled)
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_LINK_KEEP_ALIVE, strlen(WIZ_IP20_LINK_KEEP_ALIVE), HAL_MAX_DELAY);

  // Keep alive interval in milliseconds
  HAL_UART_Transmit(&huart1,
                    (uint8_t *) WIZ_IP20_LINK_KEEP_ALIVE_INTERVAL,
                    strlen(WIZ_IP20_LINK_KEEP_ALIVE_INTERVAL),
                    HAL_MAX_DELAY);

  // Keep alive retry interval in milliseconds
  HAL_UART_Transmit(&huart1,
                    (uint8_t *) WIZ_IP20_LINK_KEEP_ALIVE_RETRY_INTERVAL,
                    strlen(WIZ_IP20_LINK_KEEP_ALIVE_RETRY_INTERVAL),
                    HAL_MAX_DELAY);

  // SSL close (0 not used, 1 = enabled)
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_LINK_SSL_CLOSE, strlen(WIZ_IP20_LINK_SSL_CLOSE), HAL_MAX_DELAY);

  // Password for TCP server (0 not used)
  HAL_UART_Transmit(&huart1,
                    (uint8_t *) WIZ_IP20_TCPSRV_ENABLE_PASSWORD,
                    strlen(WIZ_IP20_TCPSRV_ENABLE_PASSWORD),
                    HAL_MAX_DELAY);
  /*
    #Password for TCP server (max 8 characters)
    HAL_UART_Transmit(&huart1,
                       (uint8_t *) WIZ_IP20_TCPSRV_SET_PASSWORD,
                       strlen(WIZ_IP20_TCPSRV_SET_PASSWORD),
                       HAL_MAX_DELAY);

    // Search id code
    HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_SEARCH_ID_CODE, strlen(WIZ_IP20_SEARCH_ID_CODE), HAL_MAX_DELAY);
  */

  // Enable debug messages (0 not used, 1 = enabled)
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_DEBUG_ENABLE, strlen(WIZ_IP20_DEBUG_ENABLE), HAL_MAX_DELAY);

  // MQTT settings (not used here)
#ifdef BLINKY_DEMO_MODE_MQTT
  // QUmqtt_user\r\n MQTT username (max 128 characters)
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_MQTT_USER, strlen(WIZ_IP20_MQTT_USER), HAL_MAX_DELAY);

  // QPmqtt_password\r\n MQTT password (max 128 characters)
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_MQTT_PASSWORD, strlen(WIZ_IP20_MQTT_PASSWORD), HAL_MAX_DELAY);

  // QCmqtt_client_id\r\n MQTT client ID (max 128 characters)
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_MQTT_CLIENT_ID, strlen(WIZ_IP20_MQTT_CLIENT_ID), HAL_MAX_DELAY);

  // QK0\r\n MQTT keep alive (0 not used, 1 = enabled)
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_MQTT_KEEP_ALIVE, strlen(WIZ_IP20_MQTT_KEEP_ALIVE), HAL_MAX_DELAY);

  // QQ0\r\n MQTT QoS (0, 1, or 2)
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_MQTT_QOS, strlen(WIZ_IP20_MQTT_QOS), HAL_MAX_DELAY);

  // Publish topic\r\n MQTT publish topic (max 128 characters)
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_MQTT_PUB_TOPIC, strlen(WIZ_IP20_MQTT_PUB_TOPIC), HAL_MAX_DELAY);

  // MQTT subscribe topic 1 (max 128 characters)
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_MQTT_SUB_TOPIC0, strlen(WIZ_IP20_MQTT_SUB_TOPIC0), HAL_MAX_DELAY);

  // MQTT subscribe topic 2 (max 128 characters)
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_MQTT_SUB_TOPIC1, strlen(WIZ_IP20_MQTT_SUB_TOPIC1), HAL_MAX_DELAY);

  // MQTT subscribe topic 3 (max 128 characters)
  HAL_UART_Transmit(&huart1, (uint8_t *) WIZ_IP20_MQTT_SUB_TOPIC2, strlen(WIZ_IP20_MQTT_SUB_TOPIC2), HAL_MAX_DELAY);

#endif

  return VSCP_ERROR_SUCCESS;
}

/*!
 * @brief  Show the current WIZnet IP20 settings by querying the module and printing the results.
 *
 * This function sends a series of AT commands to the WIZnet IP20 module to query its current configuration settings
 * (such as IP address, netmask, gateway, DNS, operation mode, etc.) and prints the responses to the console. This can
 * be useful for debugging and verifying that the module is configured correctly after initialization.
 *
 * Note: This function assumes that you have already entered command mode using goCommandMode() before calling it.
 *
 * @retval VSCP_ERROR_SUCCESS if settings were successfully queried and displayed, VSCP error code on error.
 */

int
wiznet_ip20_show_settings(void)
{
  char buf[100];
  size_t rx_len;

  LOGSTR("WIZnet IP20 current settings:\r\n");

  // Query product name
  HAL_UART_Transmit(&huart1, (uint8_t *) "MN\r\n", 4, HAL_MAX_DELAY);
  if (0 == uart1_rx_getline(buf, sizeof(buf), 500)) {
    LOGSTR("Response: %s\r\n", buf);
  }

  // Query firmware version
  HAL_UART_Transmit(&huart1, (uint8_t *) "VR\r\n", 4, HAL_MAX_DELAY);
  if (0 == uart1_rx_getline(buf, sizeof(buf), 500)) {
    LOGSTR("Response: %s\r\n", buf);
  }

  // Query MAC address
  HAL_UART_Transmit(&huart1, (uint8_t *) "MC\r\n", 4, HAL_MAX_DELAY);
  if (0 == uart1_rx_getline(buf, sizeof(buf), 500)) {
    LOGSTR("Response: %s\r\n", buf);
    // Save the MAC address
    strncpy(g_macaddrstr, buf, sizeof(g_macaddrstr) - 1);
    g_macaddrstr[sizeof(g_macaddrstr) - 1] = '\0';
  }

  // Query operation mode
  HAL_UART_Transmit(&huart1, (uint8_t *) "OP\r\n", 4, HAL_MAX_DELAY);
  if (0 == uart1_rx_getline(buf, sizeof(buf), 500)) {
    LOGSTR("Response: %s\r\n", buf);
  }

  // Get connect string
  HAL_UART_Transmit(&huart1, (uint8_t *) "SD\r\n", 4, HAL_MAX_DELAY);
  if (0 == uart1_rx_getline(buf, sizeof(buf), 500)) {
    LOGSTR("Response: %s\r\n", buf);
  }

  // Get connect string
  HAL_UART_Transmit(&huart1, (uint8_t *) "DD\r\n", 4, HAL_MAX_DELAY);
  if (0 == uart1_rx_getline(buf, sizeof(buf), 500)) {
    LOGSTR("Response: %s\r\n", buf);
  }

  // Get UART interface
  HAL_UART_Transmit(&huart1, (uint8_t *) "UN\r\n", 4, HAL_MAX_DELAY);
  if (0 == uart1_rx_getline(buf, sizeof(buf), 500)) {
    LOGSTR("Response: %s\r\n", buf);
  }

  // Get local IP address
  HAL_UART_Transmit(&huart1, (uint8_t *) "LI\r\n", 4, HAL_MAX_DELAY);
  if (0 == uart1_rx_getline(buf, sizeof(buf), 500)) {
    LOGSTR("Response: %s\r\n", buf);
    // Save the ip address
    strncpy(g_ipaddrstr, buf, sizeof(g_ipaddrstr) - 1);
    g_ipaddrstr[sizeof(g_ipaddrstr) - 1] = '\0';
  }

  // Get local subnet mask
  HAL_UART_Transmit(&huart1, (uint8_t *) "SM\r\n", 4, HAL_MAX_DELAY);
  if (0 == uart1_rx_getline(buf, sizeof(buf), 500)) {
    LOGSTR("Response: %s\r\n", buf);
  }

  // Get local gateway IP address
  HAL_UART_Transmit(&huart1, (uint8_t *) "GW\r\n", 4, HAL_MAX_DELAY);
  if (0 == uart1_rx_getline(buf, sizeof(buf), 500)) {
    LOGSTR("Response: %s\r\n", buf);
  }
  // Get local DNS IP address
  HAL_UART_Transmit(&huart1, (uint8_t *) "DS\r\n", 4, HAL_MAX_DELAY);
  if (0 == uart1_rx_getline(buf, sizeof(buf), 500)) {
    LOGSTR("Response: %s\r\n", buf);
  }

  // Get local port number
  HAL_UART_Transmit(&huart1, (uint8_t *) "LP\r\n", 4, HAL_MAX_DELAY);
  if (0 == uart1_rx_getline(buf, sizeof(buf), 500)) {
    LOGSTR("Response: %s\r\n", buf);
  }

  HAL_UART_Transmit(&huart1, (uint8_t *) "\r\n", 4, HAL_MAX_DELAY);
  if (0 == uart1_rx_getline(buf, sizeof(buf), 500)) {
    LOGSTR("Response: %s\r\n", buf);
  }

  return VSCP_ERROR_SUCCESS;
}


/*!
 * @brief  Get UART response with timeout (blocking, used during AT init only).
 * @param  buf: Buffer to store the received data.
 * @param  buf_size: Size of the buffer.
 * @param  timeout_ms: Timeout in milliseconds.
 * @retval Number of bytes received, or 0 on timeout.
 */

size_t
wiznet_ip20_getResponse(char *buf, size_t buf_size, uint32_t timeout_ms)
{
  memset(buf, 0, buf_size);
  uint16_t rx_len = 0;
  HAL_UARTEx_ReceiveToIdle(&huart1, (uint8_t *) buf, (uint16_t) (buf_size - 1), &rx_len, timeout_ms);
  if (rx_len > 0) {
    buf[rx_len] = '\0';
    LOGSTR("IP20 response: %s\r\n", buf);
  }
  return rx_len;
}