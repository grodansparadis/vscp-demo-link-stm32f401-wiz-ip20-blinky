/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <string.h>
#include <blinky.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum {
  VSCP_STATE_DISCONNECTED = 0, /**< Waiting for a client to connect    */
  VSCP_STATE_CONNECTED,        /**< Client connected, processing cmds  */
} vscp_state_t;

typedef enum {
  VSCP_SUBSTATE_POLL = 0, /**< Polling state */
  VSCP_SUBSTATE_AUTO,     /**< RETR mode */
} vscp_substate_t;

/*!
 * @brief Structure representing the persistent configuration of the node.
 *
 * This structure holds the persistent configuration data for the node.
 * The GUID, nickname, user credentials, etc are often in this storage but
 * are hardcoded in this demo.
 * The storage is used to store and retrieve configuration information that
 * should persist across device resets or power cycles.
 *
 * 'status' and 'millisecond_counter' are not persistent in this demo and
  * are not stored in flash. They are included here for completeness and to
  * illustrate how they would be part of the user registers in a full implementation.
 */

  typedef struct registers {
  uint8_t zone;                 /**< Zone for the device [P]*/
  uint8_t subzone;              /**< Subzone for the device [P] */
  // uint8_t status;               /**< Status for the device [NON PERSISTENT] */
  uint8_t control;              /**< Control for the device [P] */
  uint16_t blink_interval;      /**< Blink interval for the device [P] */
  //uint32_t millisecond_counter; /**< Millisecond counter for the device [NON PERSISTENT] */
  uint8_t button_zero_opt_byte; /**< Button option byte for the device [P] */
  uint8_t button_zone;          /**< Button zone for the device [P] */
  uint8_t button_subzone;       /**< Button subzone for the device [P] */
  uint8_t dm[32];               /**< Decision matrix for the device [P] 4 * 8 */
  // Standard registers persistent data
  uint16_t nickname;             /**< Nickname for the device (std registers) [P] */
  uint8_t userdata[5];            /**< User data for the device (std registers) [P] */
  uint8_t manufacturer_id[4];   /**< Manufacturer ID for the device (std registers) [P] */
  // NOT USE HERE, but normally used for persistent storage of GUID, user, and password
  //uint8_t guid[16];             /**< GUID for the device (std registers) [P] */
  //uint8_t user[16];             /**< Password for authentication [P] */
  //uint8_t password[32];         /**< Reserved for future use [P] */
} registers_t;

typedef union {
  registers_t data;
  // Align storage to the number of 16-bit words needed
  uint16_t word_array[sizeof(registers_t) / 2 + (sizeof(registers_t ) % 2 != 0)];
} register_union_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void
Error_Handler(void);

/* USER CODE BEGIN EFP */

/*!
  * @brief  Get a line of input from UART1 with a timeout.
  *
  * This function reads characters from UART1 into the provided output buffer
  * until a newline character is received or the specified timeout is reached.
  * The output buffer will be null-terminated. If the buffer is filled before
  * a newline is received, the function will return an error.
  *
  * @param  out: Pointer to the output buffer where the line will be stored.
  * @param  max_len: Maximum length of the output buffer (including null terminator).
  * @param  timeout_ms: Timeout in milliseconds to wait for a complete line.
  * @retval int Returns 1 if a complete line was received, 0 if a timeout occurred,
  *             or -1 if an error occurred (e.g., buffer overflow).
  */

int
uart1_rx_getline(char *out, size_t max_len, uint16_t timeout_ms);

/*!
 * @brief Set the GUID for the device.
 *
 * This function sets the GUID (Globally Unique Identifier) for the device. The GUID is a 16-byte unique identifier
 * that is used to identify the device in the VSCP network. The provided GUID should be a valid 16-byte array.
 *
 * @param pguid Pointer to a 16-byte array containing the new GUID.
 * @return VSCP_ERROR_SUCCESS on success, or an appropriate error code on failure.
 */

void
setGUID(uint8_t *const pguid);

/**
 * @brief Set defaults for the Context Defaults object
 * @param pctx Pointer to context
 */
void
resetLinkContextDefaults(vscp_link_ctx_t *pctx);


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
wiznet_ip20_command_mode(void);

/*!
 * @brief  Send a command to the WIZ-IP20 module and wait for a response.
 * @param  cmd: The command to send (null-terminated string including CRLF at end).
 * @param  response_buf: Buffer to store the response (must be pre-allocated).
 * @param  response_buf_size: Size of the response buffer.
 * @param  timeout_ms: Timeout in milliseconds to wait for the response.
 * @retval int 1 on success (response received), 0 on failure (timeout or error).
 */
int
wiznet_ip20_send_command(const char *cmd, char *response_buf, size_t response_buf_size, uint16_t timeout_ms);

/**
 * @fn validate_user
 * @brief Validate user
 *
 * @param user Username to check
 * @param password Password to check
 * @return True if user is valid, False if not.
 */
int
validate_user(const char *user, const char *password);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin        GPIO_PIN_13
#define B1_GPIO_Port  GPIOC
#define LD2_Pin       GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
/* Aliases kept for compatibility with existing user code */
#define LED_Pin       LD2_Pin
#define LED_GPIO_Port LD2_GPIO_Port

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
