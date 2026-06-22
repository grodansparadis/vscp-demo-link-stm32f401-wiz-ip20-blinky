/*
  board-config.h

  Board-specific hardware configuration for the STM32F401 + WIZnet IP20 blinky demo.

  These values MUST match the STM32CubeMX settings in stm32f401.ioc.
  When changing hardware settings in CubeMX, update the corresponding values
  in this file to keep everything in sync.

  This file intentionally has no #include directives so it can be safely
  included from both vscp-projdefs.h and blinky.h without circular dependencies.

  This file is part of VSCP - Very Simple Control Protocol
  https://www.vscp.org

  The MIT License (MIT)
  Copyright (C) 2000-2026 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
*/

#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

// ============================================================================
//  USART1 hardware configuration  (WIZnet IP20 serial interface)
//
//  Must match the CubeMX settings in stm32f401.ioc:
//    USART1 > Parameter Settings > Baud Rate / Word Length / Stop Bits /
//    Parity / Hardware Flow Control
// ============================================================================

/** Baud rate.  CubeMX: USART1 → Baud Rate */
#define USART1_BAUDRATE   115200

/** Data bits (7 or 8).  CubeMX: USART1 → Word Length */
#define USART1_DATABITS   8

/** Stop bits (1 or 2).  CubeMX: USART1 → Stop Bits */
#define USART1_STOPBITS   1

/** Parity: 0 = none, 1 = odd, 2 = even.  CubeMX: USART1 → Parity */
#define USART1_PARITY     0

/** Flow control: 0 = none, 1 = RTS, 2 = CTS, 3 = RTS/CTS.
 *  CubeMX: USART1 → Hardware Flow Control */
#define USART1_FLOWCTRL   0

// ============================================================================
//  APB1 timer clock frequency
//
//  Must match the CubeMX RCC settings in stm32f401.ioc:
//    RCC.APB1TimFreq_Value = 84000000
//
//  TIM2 and TIM3 prescalers are derived from this value at compile time.
//  If you change the PLL settings in CubeMX (PLLM/PLLN/APB1 divider),
//  update this value accordingly.
// ============================================================================

/** APB1 peripheral timer clock in Hz.  CubeMX: RCC → APB1TimFreq_Value */
#define APB1_TIM_FREQ_HZ   84000000UL

// ============================================================================
//  NVIC interrupt priorities
//
//  Priority group is NVIC_PRIORITYGROUP_4 (4 preempt bits, 0 sub bits).
//  Values here must match stm32f401.ioc NVIC settings:
//    NVIC.USART1_IRQn = true\:0\:0\:...    → preempt 0
//  TIM2 and TIM3 are not in the .ioc (added manually); their priorities
//  are defined here as the single source of truth.
// ============================================================================

/** Preempt priority for TIM2 global interrupt (1 ms tick) */
#define NVIC_PRIORITY_TIM2      0

/** Preempt priority for TIM3 global interrupt (1 µs free-running counter) */
#define NVIC_PRIORITY_TIM3      1

/** Preempt priority for USART1 global interrupt.
 *  CubeMX: NVIC → USART1_IRQn preempt priority */
#define NVIC_PRIORITY_USART1    0

// ============================================================================
//  VSCP TCP/IP link protocol port
//
//  Used by both the WIZnet IP20 module (LP command) and the VSCP link
//  protocol stack (DEFAULT_VSCP_LINK_PORT in blinky.h).
// ============================================================================

/** VSCP TCP/IP link protocol listening port */
#define VSCP_LINK_PORT    9598

#endif /* __BOARD_CONFIG_H__ */
