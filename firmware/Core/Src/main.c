/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal_gpio.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <string.h>
#include <blinky.h>
#include <wiznet-ip20.h>
#include <flash_storage.h>
#include <vscp-fifo.h>
#include <vscp-class.h>
#include <vscp-type.h>
#include <vscp-firmware-helper.h>
#include <vscp-firmware-level2.h>
#include <vscp-link-protocol.h>
#include <vscp-binary-protocol.h>
#include "vscp-link-protocol-callbacks.h"
#include "vscp-firmware-level2-callbacks.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// ** Check so one and only one build type is enabled **

/* Convert each define to 0 or 1 */
#ifdef BLINKY_MODE_TCP_SERVER
#define _MODE_TCP_SERVER 1
#else
#define _MODE_TCP_SERVER 0
#endif

#ifdef BLINKY_DEMO_MODE_TCP_CLIENT
#define _OPT_MODE_TCP_CLIENT 1
#else
#define _OPT_MODE_TCP_CLIENT 0
#endif

#ifdef BLINKY_DEMO_MODE_UDP
#define _OPT_MODE_UDP 1
#else
#define _OPT_MODE_UDP 0
#endif

#ifdef BLINKY_DEMO_MODE_MULTICAST
#define _OPT_MODE_MULTICAST 1
#else
#define _OPT_MODE_MULTICAST 0
#endif

#ifdef BLINKY_DEMO_MODE_MQTT
#define _OPT_MODE_MQTT 1
#else
#define _OPT_MODE_MQTT 0
#endif

#define _OPT_COUNT (_MODE_TCP_SERVER + _OPT_MODE_TCP_CLIENT + _OPT_MODE_UDP + _OPT_MODE_MULTICAST + _OPT_MODE_MQTT)

#if _OPT_COUNT > 1
#error                                                                                                                 \
  "Only one of BLINKY_MODE_TCP_SERVER/BLINKY_DEMO_MODE_TCP_CLIENT/BLINKY_DEMO_MODE_UDP/BLINKY_DEMO_MODE_MULTICAST/BLINKY_DEMO_MODE_MQTT may be defined at a time"
#endif

#if _OPT_COUNT != 1
#error                                                                                                                 \
  "Exactly one of BLINKY_MODE_TCP_SERVER/BLINKY_DEMO_MODE_TCP_CLIENT/BLINKY_DEMO_MODE_UDP/BLINKY_DEMO_MODE_MULTICAST/BLINKY_DEMO_MODE_MQTT must be defined"
#endif

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* Ring buffer for IRQ-driven UART1 RX ------------------------------------ */
#define UART1_RX_BUF_SIZE 2048u /* must be a power of 2 */
static uint8_t uart1_rx_buf[UART1_RX_BUF_SIZE];
static volatile uint16_t uart1_rx_head = 0; /* written by ISR/callback     */
static volatile uint16_t uart1_rx_tail = 0; /* read  by main loop          */
static uint8_t uart1_rx_byte;               /* single-byte DMA target      */

// This buffer is used to store the IP address as a string for the Wiznet IP20.
//  It is updated when we read the IP address from the module during initialization.
char g_ipaddrstr[20] = { 0 };

// This buffer stores the MAC address as a string for the Wiznet IP20
//  It is updated when we read the MAC address from the module during initialization.
char g_macaddrstr[20] = { 0 };

//----------------------------------------------------------------------------
//                      Registers (user and r/w standard)
//----------------------------------------------------------------------------

/*!
 * @brief Structure representing the persistent configuration of the node.
 *
 * This structure holds the persistent configuration data for the node.
 * The GUID, nickname, user credentials, etc are often in this storage but
 * are hardcoded in this demo.
 * The storage is used to store and retrieve configuration information that
 * should persist across device resets or power cycles.
 */

register_union_t g_registers = { .data = {
                                   .zone                 = 0,
                                   .subzone              = 0,
                                   .control              = 0b11001110, // 0b11001110
                                   .blink_interval       = 500,
                                   .button_zero_opt_byte = 0,
                                   .button_zone          = 0,
                                   .button_subzone       = 0,
                                   .manufacturer_id      = { 0x01, 0x02, 0x03, 0x04 },
                                   .dm                   = { 0 },
                                   .nickname             = 0x10,
                                   .userdata             = { 'u', 's', 'e', 'r', '\0' },
                                 } };

volatile uint8_t g_user_reg_status       = 0; // Status register for the device (std registers) [NON PERSISTENT]
volatile uint32_t g_user_reg_millisecond = 0; // Millisecond counter (std registers) [NON PERSISTENT]

//----------------------------------------------------------------------------
//                         VSCP Binary Protocol
//----------------------------------------------------------------------------

/* Binary context for each possible client connection */
static vscp_binary_ctx_t ctx_binary = { 0 };

//----------------------------------------------------------------------------
//                         VSCP Firmware Level II
//----------------------------------------------------------------------------

vscp_frmw2_ops_t ops_firmware = {
  .get_milliseconds          = vscp_frmw2_callback_get_ms,
  .get_timestamp             = vscp_frmw2_callback_get_timestamp,
  .send_event                = vscp_frmw2_callback_send_event,
  .dm_action                 = vscp_frmw2_callback_dm_action,
  .segment_ctrl_heartbeat    = vscp_frmw2_callback_segment_ctrl_heartbeat,
  .report_events_of_interest = vscp_frmw2_callback_report_events_of_interest,
  .set_event_time            = vscp_frmw2_callback_set_event_time,
  .get_ip_addr               = vscp_frmw2_callback_get_ip_addr,
  .read_reg                  = vscp_frmw2_callback_read_user_reg,
  .write_reg                 = vscp_frmw2_callback_write_user_reg,
  .stdreg_change             = vscp_frmw2_callback_stdreg_change,
  .restore_defaults          = vscp_frmw2_callback_restore_defaults,
  .enter_bootloader          = vscp_frmw2_callback_enter_bootloader,
  .reset                     = vscp_frmw2_callback_reset,
  .feed_watchdog             = vscp_frmw2_callback_feed_watchdog,
};

static vscp_link_ctx_t ctx_link;

/* Context for VSCP Level II protocol stack */
static vscp_frmw2_firmware_context_t ctx_firmware = {
  .level    = VSCP_LEVEL2,
  .state    = FRMW2_STATE_NONE,
  .substate = 0,

  .bEnableErrorReporting          = 0, // Send error reporting events (FALSE)
  .bEnableLogging                 = 0, // Enable logging events (FALSE)
  .log_id                         = 0, // Identifies log channel
  .log_level                      = 0, // Level for logs
  .bHighEndServerResponse         = 0, // React on high end server probe. Only level II (FALSE)
  .bEnableWriteProtectedLocations = 0, // GUID/manufacturer id (FALSE)
  .bUse16BitNickname              = 0, // 16-bit nickname. Default is false. Only for level I (FALSE)
  .bInterestedInAllEvents         = 1, // TRUE if interested in all events. If FALSE, only events of
                                       // interest are sent to the application on request.

  .interval_heartbeat = 60000, // Interval for heartbeats in milli-seconds (0=off)
  .last_heartbeat     = 0,     // Time for last heartbeat send
  .interval_caps      = 60000, // Interval for capabilities events in milli-seconds (0=off)
  .last_caps          = 0,     // Time for last caps send

  // Decision matrix
  .pDm         = g_registers.data.dm, // Pointer to decision matrix storage (NULL if no DM).
  .nDmRows     = 4,                   // Number of DM rows (0 if no DM).
  .sizeDmRow   = 8,                   // Size for one DM row.
  .regOffsetDm = 0xff,                // Register offset for DM (normally zero)
  .pageDm      = 0,                   // Register page where DM definition starts

  .pInternalMdf      = NULL, // Internal MDF data. Firmware can use this as needed.
  .pEventsOfInterest = NULL, // Filled in by firmware on init

  .high_end_srv_caps   = 0, // High end server capabilities
  .high_end_ip_address = 0, // High end server ip-address
  .high_end_srv_port   = 0, // High end server port

  .alarm_status               = 0,          // [I] Alarm. Read only for clients. (init=0)
  .vscp_major_version         = 1,          // [C] VSCP protocol major version. (init=1)
  .vscp_minor_version         = 4,          // [C] VSCP protocol minor version. (init=4)
  .errorCounter               = 0,          // [I] Error counter. Clear on read. Read only for clients. (init=0)
  .userId[0]                  = 11,         // [P] User id.
  .userId[1]                  = 22,         // [P] User id.
  .userId[2]                  = 33,         // [P] User id.
  .userId[3]                  = 44,         // [P] User id.
  .userId[4]                  = 55,         // [P] User id.
  .manufacturerId             = 0x01020304, // [*/P] Manufacturer id.Read only for clients.
  .manufacturerSubId          = 0x05060708, // [*/P] Manufacturer sub id.Read only for clients.
  .nickname                   = 0x10,       // [P] Device nickname (nodeid) (init=0xff)
  .page_select                = 0,          // [I] Page select register. (Init = 0)
  .firmware_major_version     = THIS_FIRMWARE_MAJOR_VERSION,   // [*] This software version. Read only for clients.
  .firmware_minor_version     = THIS_FIRMWARE_MINOR_VERSION,   // [*] This software version. Read only for clients.
  .firmware_sub_minor_version = THIS_FIRMWARE_RELEASE_VERSION, // [*] This software version. Read only for clients.
  .bootloader_algorithm       = 0,                             // [*] Boot loader algorithm we use.
  .standard_device_family     = 0x0002,                        // [*] Family code. Read only for clients.
  .standard_device_type       = 0,                             // [*] Family type. Read only for clients.
  .firmware_device_code       = 0x0001,                        // [*] Firmware device code. Read only for clients.

  .guid       = { 0 },                        // GUID is set from STM32 unique ID during initialization
  .mdfurl     = "eurosource.se/blinky0.json", // Max 32 bytes
  .ipaddr     = { 0 },
  .deviceName = "VSCP blinky tcp/ip link demo", // Max 64 bytes

  .ops       = &ops_firmware,
  .puserdata = &ctx_link, // Link protocol context is available as user data for firmware callbacks
};

//----------------------------------------------------------------------------
//                         VSCP Link Protocol
//----------------------------------------------------------------------------

/*
  Connect callbacks in op table. This table is passed to the VSCP link protocol handler
  and defines the behavior for various operations.
*/
static const vscp_link_ops_t link_ops = {
  /* ── Transport ──────────────────────────────────────────────── */
  .write_client = vscp_link_callback_write_client,
  .disconnect   = vscp_link_callback_disconnect,
  .quit         = vscp_link_callback_quit,
  .welcome      = vscp_link_callback_welcome,

  /* ── Authentication ─────────────────────────────────────────── */
  .check_user          = vscp_link_callback_check_user,
  .check_password      = vscp_link_callback_check_password,
  .challenge           = vscp_link_callback_challenge,
  .check_authenticated = vscp_link_callback_check_authenticated,
  .check_privilege     = vscp_link_callback_check_privilege,

  /* ── Event I/O ──────────────────────────────────────────────── */
  .send               = vscp_link_callback_send,
  .chkdata            = vscp_link_callback_chkData,
  .clrall             = vscp_link_callback_clrall,
  .retr               = vscp_link_callback_retr,
  .rcvloop            = vscp_link_callback_rcvloop,
  .enable_rcvloop     = vscp_link_callback_enable_rcvloop,
  .get_rcvloop_status = vscp_link_callback_get_rcvloop_status,

  /* ── Node information ───────────────────────────────────────── */
  .get_guid       = vscp_link_callback_get_guid,
  .set_guid       = vscp_link_callback_set_guid,
  .get_version    = vscp_link_callback_get_version,
  .get_channel_id = vscp_link_callback_get_channel_id,
  .wcyd           = vscp_link_callback_wcyd,
  .statistics     = vscp_link_callback_statistics,
  .info           = vscp_link_callback_info,

  /* ── Filter / buffer ────────────────────────────────────────── */
  .set_filter = vscp_link_callback_setFilter,
  .set_mask   = vscp_link_callback_setMask,

  /* ── Interfaces ─────────────────────────────────────────────── */
  .get_interface_count = vscp_link_callback_get_interface_count,
  .get_interface       = vscp_link_callback_get_interface,
  .close_interface     = vscp_link_callback_close_interface,

  /* ── Misc ────────────────────────────────────────────────────── */
  .help_custom = NULL, // OR vscp_link_callback_help,
  .test        = vscp_link_callback_test,
  .shutdown    = vscp_link_callback_shutdown,
  .restart     = vscp_link_callback_restart,

  /* ── Binary mode ─────────────────────────────────────────────── */
  .binary = NULL // vscp_link_callback_binary,
};

/* Context for each possible client connection  */
static vscp_link_ctx_t ctx_link = {
  .id                = 0,
  .next              = NULL,
  .ops               = &link_ops,
  .sock              = VSCP_STATE_DISCONNECTED, // Non zero when connected
  .guid              = { 0 },
  .user              = { 0 },
  .fifoEventsIn      = { 0 }, // VSCP event receive fifo (from client)
  .fifoEventsOut     = { 0 }, // VSCP event transmit fifo (to client)
  .bValidated        = 0,
  .bRcvLoop          = 0,
  .privLevel         = 0,
  .filter            = { 0 },
  .statistics        = { 0 },
  .status            = { 0 },
  .last_rcvloop_time = 0,
  .user              = { 0 }, // Usename is stored here while waiting for password (32 bytes)
  .user_data         = &ctx_firmware,
};

node_persistent_config_t g_persistent;

volatile uint32_t msTicks = 0U; /* incremented every 1 ms by TIM2 ISR */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void
SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void
uart1_rx_start(void);
// int
// uart1_rx_getline(char *out, size_t max_len, uint16_t timeout_ms);
static int
uart1_rx_wait_for(const char *needle, uint16_t timeout_ms);
static int
uart1_rx_scan(const char *needle);
static void
uart1_rx_consume_through(const char *needle);
void
setLinkContextDefaults(vscp_link_ctx_t *pctx);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include <stdio.h>
#include "usart.h"

/*!
 * @brief  HAL callback – fires after each byte received via UART1 IT.
 *         Stores the byte in the ring buffer and re-arms the IRQ.
 */

/*!
 * @brief  EXTI callback – fires when PC13 (B1 button) changes state.
 *
 *         The button is active-low: pin reads 0 when pressed, 1 when released.
 *         Both edges are enabled in GPIO_MODE_IT_RISING_FALLING, so this
 *         callback is invoked on press and release.
 *
 * @param  GPIO_Pin  Pin that triggered the interrupt (should be B1_Pin).
 */
void
HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == B1_Pin) {

    // Button bounce protection (also 100nF cap)
    static uint32_t last_edge_ms = 0;
    uint32_t now                 = HAL_GetTick();
    if ((now - last_edge_ms) < 30u) {
      return; /* ignore bounce within 30 ms window */
    }
    last_edge_ms = now;

    if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {

      LOGSTR("Button pressed (pin went low)\r\n");

      // Set the button active bit in status register
      g_user_reg_status |= BLINKY_STATUS_BTN_ON;

      // If enabled Send a VSCP Turn-ON event to indicate the button press.
      // https://grodansparadis.github.io/vscp-doc-spec/#/./class1.control?id=type5
      if (g_registers.data.control & BLINKY_CTRL_ENABLE_BTN_TURNON) {

        vscp_event_t *pev = vscp_fwhlp_newEvent();
        if (NULL == pev) {
          return;
        }

        if (VSCP_ERROR_SUCCESS != vscp_frmw2_setup_event(&ctx_firmware, pev, 3)) {
          vscp_fwhlp_deleteEvent(&pev);
          return;
        }

        pev->vscp_class = VSCP_CLASS1_CONTROL;
        pev->vscp_type  = VSCP_TYPE_CONTROL_TURNON;
        pev->pdata[0]   = g_registers.data.button_zero_opt_byte;
        pev->pdata[1]   = g_registers.data.button_zone;
        pev->pdata[2]   = g_registers.data.button_subzone;

        // Add event to the outgoing FIFO for the link protocol to send to client
        if (!vscp_fifo_write(&ctx_link.fifoEventsOut, pev)) {
          LOGSTR("Failed to enqueue button press event\r\n");
          vscp_fwhlp_deleteEvent(&pev);
        }
      }

      // If enabled Send a VSCP START event to indicate the button down.
      // https://grodansparadis.github.io/vscp-doc-spec/#/./class1.information?id=type25
      if (g_registers.data.control & BLINKY_CTRL_ENABLE_BTN_START) {

        vscp_event_t *pev = vscp_fwhlp_newEvent();
        if (NULL == pev) {
          return;
        }

        if (VSCP_ERROR_SUCCESS != vscp_frmw2_setup_event(&ctx_firmware, pev, 3)) {
          vscp_fwhlp_deleteEvent(&pev);
          return;
        }

        pev->vscp_class = VSCP_CLASS1_INFORMATION;
        pev->vscp_type  = VSCP_TYPE_INFORMATION_START;
        pev->pdata[0]   = 0; // reserved for future use;
        pev->pdata[1]   = g_registers.data.button_zone;
        pev->pdata[2]   = g_registers.data.button_subzone;

        // Add event to the outgoing FIFO for the link protocol to send to client
        if (!vscp_fifo_write(&ctx_link.fifoEventsOut, pev)) {
          LOGSTR("Failed to enqueue button start event\r\n");
          vscp_fwhlp_deleteEvent(&pev);
        }
      }
    }
    else {
      LOGSTR("Button released (pin went high)\r\n");

      // Clear the button active bit in status register
      g_user_reg_status &= ~BLINKY_STATUS_BTN_ON;

      // If enabled Send a VSCP STOP event to indicate the button release.
      // https://grodansparadis.github.io/vscp-doc-spec/#/./class1.information?id=type24
      if (g_registers.data.control & BLINKY_CTRL_ENABLE_BTN_STOP) {

        vscp_event_t *pev = vscp_fwhlp_newEvent();
        if (NULL == pev) {
          return;
        }

        if (VSCP_ERROR_SUCCESS != vscp_frmw2_setup_event(&ctx_firmware, pev, 3)) {
          vscp_fwhlp_deleteEvent(&pev);
          return;
        }

        pev->vscp_class = VSCP_CLASS1_INFORMATION;
        pev->vscp_type  = VSCP_TYPE_INFORMATION_STOP;
        pev->pdata[0]   = 0; // reserved for future use;
        pev->pdata[1]   = g_registers.data.button_zone;
        pev->pdata[2]   = g_registers.data.button_subzone;

        // Add event to the outgoing FIFO for the link protocol to send to client
        if (!vscp_fifo_write(&ctx_link.fifoEventsOut, pev)) {
          LOGSTR("Failed to enqueue button release event\r\n");
          vscp_fwhlp_deleteEvent(&pev);
        }
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////
// Retargeting printf to USART2 for debug output.
// This allows us to use LOGSTR() to send data over USART2,
// which can be useful for debugging purposes. The __io_putchar
// function is called by the LOGSTR() function to output each character,
// and it uses the HAL_UART_Transmit function to send the character over
// USART2. Make sure that USART2 is properly initialized and configured in
// your project for this to work correctly.
//

/*!
 * @brief  Retargets the C library printf function to the USART.
 * @param  ch: Character to be transmitted.
 * @retval The character that was transmitted.
 */

int
__io_putchar(int ch)
{
  HAL_UART_Transmit(&huart2, (uint8_t *) &ch, 1, HAL_MAX_DELAY);
  return ch;
}

/*!
 * @brief  Retargets the C library printf function to the USART.
 * @param  file: File descriptor (not used).
 * @param  ptr: Pointer to the data to be written.
 * @param  len: Length of the data to be written.
 * @retval Number of bytes written.
 */

int
_write(int file, char *ptr, int len)
{
  for (int i = 0; i < len; i++) {
    __io_putchar(*ptr++);
  }
  return len;
}

/*!
 * @brief  Re-arm UART1 IRQ for one byte.  Call once at startup and again
 *         from HAL_UART_RxCpltCallback.
 */

static void
uart1_rx_start(void)
{
  HAL_UART_Receive_IT(&huart1, &uart1_rx_byte, 1);
}

void
HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1) {
    uint16_t next = (uart1_rx_head + 1u) & (UART1_RX_BUF_SIZE - 1u);
    if (next != uart1_rx_tail) { /* drop byte if buffer full */
      uart1_rx_buf[uart1_rx_head] = uart1_rx_byte;
      uart1_rx_head               = next;
    }
    uart1_rx_start(); /* re-arm for next byte     */
  }
}

/*!
 * @brief  Fetch a \r\n-terminated string from the UART1 ring buffer.
 *
 * The buffer is only consumed when a complete \r\n line is available,
 * so partial data is never discarded on timeout.
 *
 * @param  out        Destination buffer; null-terminated on success.
 * @param  max_len    Size of @p out in bytes (including the null terminator).
 * @param  timeout_ms Wait behaviour:
 *                      0x0000 – return immediately if no complete line yet
 *                      0x0001…0xFFFE – wait up to N milliseconds
 *                      0xFFFF – wait forever
 * @retval  0   A complete \r\n-terminated line was copied into @p out.
 * @retval -1   No complete line available within the specified timeout.
 */

int
uart1_rx_getline(char *out, size_t max_len, uint16_t timeout_ms)
{
  /* Scan the ring buffer for \r\n without consuming any bytes yet. */
  for (;;) {
    /* --- peek: is there a \r\n somewhere in the buffer? --- */
    uint16_t head  = uart1_rx_head; /* snapshot (volatile, read once) */
    uint16_t idx   = uart1_rx_tail;
    uint8_t found  = 0;
    uint16_t eol   = 0; /* index of '\n' in ring buffer   */
    uint16_t count = 0; /* bytes up to and including '\n' */

    while (idx != head) {
      uint8_t ch = uart1_rx_buf[idx];
      count++;
      if (ch == '\n' && count >= 2 && uart1_rx_buf[(idx - 1u) & (UART1_RX_BUF_SIZE - 1u)] == '\r') {
        found = 1;
        eol   = idx;
        break;
      }
      idx = (idx + 1u) & (UART1_RX_BUF_SIZE - 1u);
    }

    if (found) {
      /* Consume bytes up to and including the '\n'. */
      size_t pos = 0;
      idx        = uart1_rx_tail;
      while (1) {
        uint8_t ch = uart1_rx_buf[idx];
        if (pos < max_len - 1u) {
          out[pos++] = (char) ch;
        }
        idx           = (idx + 1u) & (UART1_RX_BUF_SIZE - 1u);
        uart1_rx_tail = idx;
        if (idx == (uint16_t) ((eol + 1u) & (UART1_RX_BUF_SIZE - 1u)))
          break;
      }
      out[pos] = '\0';
      return 0;
    }

    /* No complete line yet – handle timeout. */
    if (timeout_ms == 0u) {
      return -1; /* no-wait: fail immediately      */
    }
    if (timeout_ms == 0xFFFFu) {
      continue; /* wait forever: keep polling     */
    }

    /* Finite timeout: spin until deadline or line arrives. */
    uint32_t deadline = HAL_GetTick() + timeout_ms;
    while (HAL_GetTick() < deadline) {
      /* re-check ring buffer */
      head  = uart1_rx_head;
      idx   = uart1_rx_tail;
      found = 0;
      count = 0;
      while (idx != head) {
        uint8_t ch = uart1_rx_buf[idx];
        count++;
        if (ch == '\n' && count >= 2 && uart1_rx_buf[(idx - 1u) & (UART1_RX_BUF_SIZE - 1u)] == '\r') {
          found = 1;
          eol   = idx;
          break;
        }
        idx = (idx + 1u) & (UART1_RX_BUF_SIZE - 1u);
      }
      if (found)
        break;
    }
    if (!found)
      return -1; /* timed out                      */
    /* line arrived during finite wait – loop back to consume it        */
    timeout_ms = 0xFFFFu; /* force consume path next iter   */
  }
}

/*!
 * @brief  Wait until @p needle appears anywhere in a received line.
 *
 * Reads complete \r\n-terminated lines from the ring buffer one at a time
 * and checks whether @p needle is a substring of each line.
 * Lines that do not match are discarded.
 *
 * @param  needle      String to search for (case-sensitive substring match).
 * @param  timeout_ms  Same semantics as uart1_rx_getline:
 *                       0x0000 – return immediately if no matching line queued
 *                       0x0001–0xFFFE – wait up to N ms
 *                       0xFFFF – wait forever
 * @retval  0   A line containing @p needle was found.
 * @retval -1   Timeout elapsed without a matching line.
 */

static int __attribute__((unused))
uart1_rx_wait_for(const char *needle, uint16_t timeout_ms)
{
  static char line[100];
  uint32_t deadline = (timeout_ms != 0u && timeout_ms != 0xFFFFu) ? HAL_GetTick() + timeout_ms : 0u;

  for (;;) {
    /* How long to wait for the next line? */
    uint16_t line_timeout;
    if (timeout_ms == 0u) {
      line_timeout = 0u; /* no-wait              */
    }
    else if (timeout_ms == 0xFFFFu) {
      line_timeout = 0xFFFFu; /* wait forever         */
    }
    else {
      uint32_t now = HAL_GetTick();
      if (now >= deadline)
        return -1; /* overall timeout      */
      uint32_t left = deadline - now;
      line_timeout  = (left > 0xFFFEu) ? 0xFFFEu : (uint16_t) left;
    }

    if (uart1_rx_getline(line, sizeof(line), line_timeout) != 0) {
      return -1; /* no line / timed out  */
    }

    if (strstr(line, needle) != NULL) {
      return 0; /* match found          */
    }
    /* line didn't match – discard and try again (unless no-wait) */
    if (timeout_ms == 0u)
      return -1;
  }
}

/*!
 * @brief  Scan the ring buffer for @p needle without consuming any bytes.
 * @retval 0  Needle found in the buffer.
 * @retval -1 Needle not present.
 */

static int
uart1_rx_scan(const char *needle)
{
  size_t nlen   = strlen(needle);
  uint16_t head = uart1_rx_head; /* snapshot */
  uint16_t idx  = uart1_rx_tail;
  size_t mpos   = 0; /* match position within needle */

  while (idx != head) {
    if ((char) uart1_rx_buf[idx] == needle[mpos]) {
      mpos++;
      if (mpos == nlen)
        return 0; /* full match */
    }
    else {
      mpos = 0;
      /* re-check current char against needle[0] */
      if ((char) uart1_rx_buf[idx] == needle[0])
        mpos = 1;
    }
    idx = (idx + 1u) & (UART1_RX_BUF_SIZE - 1u);
  }
  return -1;
}

/*!
 * @brief  Consume bytes from the ring buffer up to and including the first occurrence of @p needle.
 *
 * This function scans the ring buffer for the first occurrence of @p needle (a string), and if found, advances the
 * tail pointer to consume all bytes up to and including that occurrence. If @p needle is not found, the buffer is
 * left unchanged.
 *
 * @param  needle  String to search for (case-sensitive substring match).
 */
static void
uart1_rx_consume_through(const char *needle)
{
  size_t nlen   = strlen(needle);
  uint16_t head = uart1_rx_head; /* snapshot */
  uint16_t idx  = uart1_rx_tail;
  size_t mpos   = 0;

  while (idx != head) {
    if ((char) uart1_rx_buf[idx] == needle[mpos]) {
      mpos++;
      idx = (idx + 1u) & (UART1_RX_BUF_SIZE - 1u);
      if (mpos == nlen) {
        uart1_rx_tail = idx; /* consume up to end of needle */
        return;
      }
    }
    else {
      mpos = 0;
      if ((char) uart1_rx_buf[idx] == needle[0]) {
        mpos = 1;
      }
      idx = (idx + 1u) & (UART1_RX_BUF_SIZE - 1u);
    }
  }
}

/*!
 * @brief  Get the unique identifier and set (GUID) for the device.
 *
 * VSCP reserves a group of GUID's for MCU's with a built-in unique ID,
 * where the GUID is constructed from a fixed prefix.
 *
 * One other possibility was to use the WIZnet's MAC address and the
 * reserved GUID prefix to construct a GUID built here we use the unique
 * MCU ID instead since it's guaranteed to be unique and doesn't require
 * an extra step to read the MAC address from the WIZnet.
 *
 * More info on VSCP GUID construction for MCU-based devices here:
 * https://grodansparadis.github.io/vscp-doc-spec/#/./vscp_globally_unique_identifiers
 *
 *This GUID is constructed like this
 *
 * FD:AA:BB:YY:YY:YY:YY:YY:YY:YY:YY:YY:YY:YY:YY:YY
 *
 * AA is a manufacturer code assigned by VSCP (02 for STMicroelectronics)
 *
 * BB is a device type code assigned by VSCP (01 for STM32)
 *
 * The 13 byte YY:YY:YY:YY:YY:YY:YY:YY:YY:YY:YY:YY:YY is something we can
 * define freely as long as it's unique for each device. We can get 12 byte
 * from the unique MCU ID using the HAL_GetUIDw0(), HAL_GetUIDw1(),
 * and HAL_GetUIDw2() functions. Then we can set the last byte to zero.
 *
 * @param  pguid: Pointer to a 16-byte array where the GUID will be stored.
 */

void
setGUID(uint8_t *const pguid)
{
  uint32_t uid;

  memset(pguid, 0, 16); /* Clear the GUID buffer */
  pguid[0] = 0xFD;      /* VSCP reserved prefix for MCU-based GUIDs */
  pguid[1] = 0x00;      /* Manufacturer code for STMicroelectronics */
  pguid[2] = 0x02;

  uid      = HAL_GetUIDw0(); // Words 0 (bits 31:0)
  pguid[3] = (uid >> 24) & 0xFF;
  pguid[4] = (uid >> 16) & 0xFF;
  pguid[5] = (uid >> 8) & 0xFF;
  pguid[6] = uid & 0xFF;

  uid       = HAL_GetUIDw1(); // Words 1 (bits 63:32)
  pguid[7]  = (uid >> 24) & 0xFF;
  pguid[8]  = (uid >> 16) & 0xFF;
  pguid[9]  = (uid >> 8) & 0xFF;
  pguid[10] = uid & 0xFF;

  uid       = HAL_GetUIDw2(); // Words 2 (bits 95:64)
  pguid[11] = (uid >> 24) & 0xFF;
  pguid[12] = (uid >> 16) & 0xFF;
  pguid[13] = (uid >> 8) & 0xFF;
  pguid[14] = uid & 0xFF;

  pguid[15] = ctx_firmware.nickname; // No nickname discobery for level II node
}

/*!
 * @brief  Reset the VSCP link context to default values for a new connection.
 * @param  pctx: Pointer to the context structure to reset.
 *
 * This function is similar to setLinkContextDefaults but is intended to be called
 * when resetting the context for a new client connection, without changing
 * fields that should persist across connections (like the GUID).
 *
 * Note: In this simple example we only support one client at a time, so we
 * don't actually need to preserve any fields across connections, but in a more
 * complex implementation with multiple clients you might want to keep certain
 * fields unchanged when resetting for a new connection.
 */

void
resetLinkContextDefaults(vscp_link_ctx_t *pctx)
{
  pctx->bValidated = 0; // No credentials yet
  pctx->privLevel  = 0; // No privileges before we are logged in
  pctx->bRcvLoop   = 0; // Polling mode by default, can switch to RETR after login if desired
  memset(&pctx->filter, 0, sizeof(vscpEventFilter));       // All events is received by client
  memset(&pctx->statistics, 0, sizeof(vscp_statistics_t)); // VSCP Statistics
  memset(&pctx->status, 0, sizeof(vscp_status_t));         // VSCP status
  pctx->last_rcvloop_time = 0;

  // Drain the fifos
  vscp_fifo_clear(&pctx->fifoEventsOut);
  vscp_fifo_clear(&pctx->fifoEventsIn);
}

///////////////////////////////////////////////////////////////////////////////
// validate_user
//
int
validate_user(const char *user, const char *password)
{
  // For demonstration purposes, we accept a single hardcoded username/password.
  // In a real implementation, you should check against securely stored credentials.
  const char *valid_user     = BLINKY_DEFAULT_VSCP_LINK_USER;
  const char *valid_password = BLINKY_DEFAULT_VSCP_LINK_PASSWORD;

  return (strcmp(user, valid_user) == 0) && (strcmp(password, valid_password) == 0);
}

/* USER CODE END 0 */

///////////////////////////////////////////////////////////////////////////////
// main
//

int
main(void)
{
  /* USER CODE BEGIN 1 */
  int rv;
  char buf[2672]; // Buffer for cmd responses and incoming data and event conversions
  uint32_t now;
  //size_t rx_len;
  //(void) rx_len; // only used to capture AT response lengths during init
  // uint32_t led_blink_until    = 0; // LED blink timestamp
  // uint32_t heartbeat_interval = 0; // Heartbeat clock
  // vscp_state_t vscp_substate  = VSCP_SUBSTATE_POLL;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* Start TIM2 in interrupt mode — fires every 1 ms */
  if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK) {
    Error_Handler();
  }

  /* Start TIM3 — free-running 1 µs counter, overflow interrupt for 32-bit extension */
  if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK) {
    Error_Handler();
  }

  // Switch to IRQ-driven receive for data mode
  uart1_rx_start();

  // Initialize context for client connections
  setGUID(ctx_link.guid);

  // Both fifos empty at startup, but we initialize them here to ensure they are ready for use
  vscp_fifo_init(&ctx_link.fifoEventsOut, BLINKY_OUTGOING_FIFO_SIZE);
  vscp_fifo_init(&ctx_link.fifoEventsIn, BLINKY_INCOMING_FIFO_SIZE);

  /*
    * WIZ-IP20 initialization sequence (AT command mode, blocking)
      We use blocking calls here for simplicity since this only runs once at startup.

      Commands are here: https://docs.wiznet.io/Product/Modules/Serial-to-Ethernet-Module/W232N/command-manual-en
  */

  // rx_len = getWizIp20Response(buf, sizeof(buf), 10);

  wiznet_ip20_enter_command_mode();
  wiznet_ip20_init();
  wiznet_ip20_show_settings();
  wiznet_ip20_save();
  wiznet_ip20_restart();

  // Initialize the VSCP firmware stack with the context for our single client connection
  vscp_frmw2_init(&ctx_firmware);

  // Debug print to indicate that the program has started
  LOGSTR("STM32F401 Wiznet IP20 VSCP blink demo starting\r\n");

  // if (HAL_OK != flash_storage_write(0, g_registers.word_array, sizeof(register_union_t))) {
  //     LOGSTR("Failed to write flash storage\r\n");
  //   }

  if (!flash_storage_is_valid()) {

    // Handle invalid flash storage
    LOGSTR("Flash storage is invalid. Erase and initializing to default values.\r\n");

    if (HAL_OK != flash_storage_erase()) {
      LOGSTR("Failed to erase flash storage\r\n");
    }

    if (HAL_OK != flash_storage_write_header()) {
      LOGSTR("Failed to initialize flash storage\r\n");
    }

    if (HAL_OK != flash_storage_write(FLASH_STORAGE_DATA_OFFSET,
                                      g_registers.word_array,
                                      sizeof(register_union_t) / sizeof(uint16_t))) {
      LOGSTR("Failed to write flash storage\r\n");
    }
  }
  else {
    LOGSTR("Flash storage is valid. Reading data into registers.\r\n");
  }

  flash_storage_read(FLASH_STORAGE_DATA_OFFSET, g_registers.word_array, sizeof(register_union_t) / sizeof(uint16_t));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // const vscpEventEx *const pex
    if (vscp_frmw2_work(&ctx_firmware, NULL)) {
      /* If an event was produced by the firmware, we can handle it here (e.g. add to outgoing FIFO) */
      LOGSTR("Firmware produced an event\r\n");
    }

    /* ------------------------------------------------------------------
     * State machine – scan ring buffer for control tokens (no \r\n needed)
     * ----------------------------------------------------------------*/
    switch (ctx_link.sock) {

      case VSCP_STATE_DISCONNECTED:
        if (uart1_rx_scan("<CONNECT>") == 0) {
          uart1_rx_consume_through("<CONNECT>");
          LOGSTR("Received <CONNECT>\r\n");
          //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
          // led_blink_until = HAL_GetTick() + 50u;
          ctx_link.sock = VSCP_STATE_CONNECTED;
          LOGSTR("State: DISCONNECTED -> CONNECTED\r\n");
          vscp_link_connect(&ctx_link);
        }
        else {
          /* Discard any \r\n-terminated lines that aren't <CONNECT> */
          uart1_rx_getline(buf, sizeof(buf), 0);
        }
        break;

      case VSCP_STATE_CONNECTED:
        if (uart1_rx_scan("<DISCONNECT>") == 0) {
          uart1_rx_consume_through("<DISCONNECT>");
          LOGSTR("Received <DISCONNECT>\n");
          //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
          // led_blink_until = HAL_GetTick() + 50u;
          HAL_UART_Transmit(&huart1, (uint8_t *) "+OK - Disconnect.\r\n", 19, HAL_MAX_DELAY);
          ctx_link.sock = VSCP_STATE_DISCONNECTED;
          LOGSTR("State: CONNECTED -> DISCONNECTED\r\n");
        }
        else if (uart1_rx_getline(buf, sizeof(buf), 0) == 0) {
          /* Consume and log incoming VSCP commands */
          LOGSTR("VSCP cmd: %s", buf);
          
          vscp_link_parser(&ctx_link, buf);
        }
        else if (ctx_link.bRcvLoop && ctx_link.bValidated) {
          // Check if there is events for the client
          vscpEvent *pev = NULL;
          // Check if there is an event in the out queue
          if (vscp_fifo_read(&ctx_link.fifoEventsOut, &pev)) {
            // We should have a valid pointer to an event here
            if (NULL == pev) {
              break;
            }
            // Convert to string format
            if (VSCP_ERROR_SUCCESS == (rv = vscp_fwhlp_eventToString(buf, sizeof(buf), pev))) {
              strcat(buf, "\r\n");
              ctx_link.ops->write_client(&ctx_link, buf);
            }
            else {
              LOGSTR("Failed to convert event to string: %d\r\n", rv);
            }
            // Free event
            vscp_fwhlp_deleteEvent(&pev);

            // Update last receive loop time to avoid sending alive marker when not needed
            ctx_link.last_rcvloop_time = HAL_GetTick();
          }
          else {
            // No event in out fifo, should we write alive marker
            if (HAL_GetTick() - ctx_link.last_rcvloop_time > 1000) {
              ctx_link.last_rcvloop_time = HAL_GetTick();
              ctx_link.ops->write_client(&ctx_link, "+OK\r\n");
            }
          }
        }

        break;

      default:
        ctx_link.sock = VSCP_STATE_DISCONNECTED;
        break;
    }

    // * * * LED BLINK * * *
    /* 
      Toggle the state of pin 13 on GPIO port C
      Blink if enabled in control register and if
      blink interval is set to a non-zero value. 
      The blink interval is in milliseconds.
    */
    if (g_registers.data.blink_interval && (g_registers.data.control & BLINKY_CTRL_ENABLE_LED)) {
      if ((HAL_GetTick() - now) >= g_registers.data.blink_interval) {
        // Toggle the LED pin
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);  
        now = HAL_GetTick();
        // Toggle LED active bit in status register
        g_user_reg_status ^= BLINKY_STATUS_LED_ON;
      }
    }
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void
SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
  RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   * HSE 8 MHz -> PLL -> SYSCLK 84 MHz
   * PLLM=4, PLLN=84, PLLP=2 => VCO=168 MHz / 2 = 84 MHz
   * PLLQ=7 => USB/SDIO/RNG = 168/7 = 24 MHz
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState       = RCC_HSE_BYPASS; /* External clock on PH0 */
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM       = 4;
  RCC_OscInitStruct.PLL.PLLN       = 84;
  RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ       = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1; /* HCLK  = 84 MHz */
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;   /* APB1  = 42 MHz (timers 84 MHz) */
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;   /* APB2  = 84 MHz */

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void
Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void
assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: LOGSTR("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
