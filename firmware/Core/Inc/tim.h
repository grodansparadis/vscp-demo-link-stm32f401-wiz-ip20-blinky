/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
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
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern volatile uint32_t tim3_ovf;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_TIM2_Init(void);
void MX_TIM3_Init(void);

/* USER CODE BEGIN Prototypes */

/**
 * @brief  Read the 32-bit 1 µs free-running counter.
 *
 * TIM3 (16-bit hardware) is extended to 32 bits via a software overflow
 * counter (tim3_ovf) incremented in the TIM3 update ISR.
 * The read sequence is overflow-safe: if an overflow occurs between
 * reading hi and lo, the second hi read will differ and lo is re-read.
 *
 * Range:  effectively unbounded (64-bit @ 1 µs/tick wraps after ~584 542 years).
 *
 * @return Microseconds since TIM3 was started.
 */
static inline uint64_t usec_now(void)
{
  uint32_t hi, lo;
  do {
    hi = tim3_ovf;
    lo = TIM3->CNT;
  } while (tim3_ovf != hi); /* retry if overflow hit between reads */
  return ((uint64_t)hi << 16) | lo;
}

/**
 * @brief  Enable the DWT cycle counter.
 *
 * Must be called once at startup (after SystemClock_Config) before
 * nsec_now() is used.
 */
void dwt_init(void);

/**
 * @brief  Nanosecond timestamp (TIM3 µs base + DWT sub-µs).
 *
 * Combines the overflow-safe 64-bit TIM3 microsecond counter with the
 * DWT cycle counter for sub-microsecond resolution.
 *
 * Resolution : 1 / SystemCoreClock  (~11.9 ns at 84 MHz).
 * Accuracy   : ±1 µs at the µs boundary (TIM3 and DWT are not synchronised).
 *              Duration measurements between two calls cancel this error.
 * Range      : same as usec_now() — effectively unlimited.
 *
 * @return Nanoseconds since TIM3 was started.
 */
static inline uint64_t nsec_now(void)
{
  uint32_t cyc_per_us = SystemCoreClock / 1000000UL;
  uint32_t hi, lo, cyc;
  do {
    hi  = tim3_ovf;
    lo  = TIM3->CNT;
    cyc = DWT->CYCCNT;
  } while (tim3_ovf != hi); /* retry if TIM3 overflow hit between reads */
  uint64_t us  = ((uint64_t)hi << 16) | lo;
  uint32_t sub = (cyc % cyc_per_us) * 1000UL / cyc_per_us; /* 0–999 ns */
  return us * 1000ULL + sub;
}

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

