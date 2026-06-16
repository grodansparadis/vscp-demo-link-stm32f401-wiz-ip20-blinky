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

/**
 * @brief Set defaults for the Context Defaults object
 * @param pctx Pointer to context
 */
void
setContextDefaults(vscp_link_ctx_t *pctx);

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
