/**
 ******************************************************************************
 * @file    watchdog.c
 * @brief   Independent Watchdog (IWDG) enable / feed / disable helpers.
 *
 * Timeout: 10 seconds
 *   LSI ≈ 32 000 Hz, prescaler /256 → 125 Hz, reload 1249 → 10.0 s
 *
 * IMPORTANT: The IWDG cannot be stopped by software once started.
 * watchdog_disable() clears only the software feed-gate.
 ******************************************************************************
 */

#include "watchdog.h"

/* Private variables ---------------------------------------------------------*/
static IWDG_HandleTypeDef hiwdg;

/**
 * Software feed-gate.
 * 0 = feeding suspended (watchdog still running in hardware)
 * 1 = feeding active
 */
static volatile uint8_t g_watchdog_enabled = 0U;

/* Public functions ----------------------------------------------------------*/

void
watchdog_enable(void)
{
#ifdef DEBUG
  /*
   * Debug build: watchdog is disabled entirely so the debugger can halt
   * the CPU without triggering a reset.
   * g_watchdog_enabled stays 0; watchdog_feed() is a no-op.
   */
  (void) hiwdg; /* suppress unused-variable warning */
#else
  /*
   * Release build: freeze IWDG counter while the core is halted by a
   * debugger (harmless when no debugger is attached).
   */
  __HAL_DBGMCU_FREEZE_IWDG();

  hiwdg.Instance       = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256; /* LSI/256 → 125 Hz         */
  hiwdg.Init.Reload    = 1249U;              /* 1250 ticks / 125 Hz = 10 s */

  if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
    Error_Handler();
  }

  g_watchdog_enabled = 1U;
#endif /* DEBUG */
}

void
watchdog_disable(void)
{
  /*
   * The IWDG hardware cannot be stopped once started.
   * Clearing the gate means watchdog_feed() becomes a no-op, so the MCU
   * will reset ~10 s after the last successful feed.
   */
  g_watchdog_enabled = 0U;
}

void
watchdog_feed(void)
{
  if (g_watchdog_enabled) {
    HAL_IWDG_Refresh(&hiwdg);
  }
}

int
watchdog_is_enabled(void)
{
  return (int) g_watchdog_enabled;
}
