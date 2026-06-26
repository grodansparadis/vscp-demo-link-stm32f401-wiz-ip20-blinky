/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/*
 * Independent Watchdog (IWDG) helper
 * ===================================
 * The IWDG is clocked from the LSI RC oscillator (~32 kHz) and is fully
 * independent of the CPU clock.  Once started it CANNOT be stopped by
 * software – this is a hardware-enforced safety feature on STM32F4.
 *
 * Timeout configuration (10 s):
 *   LSI  ≈ 32 000 Hz
 *   Prescaler /256 → IWDG clock = 32 000 / 256 = 125 Hz
 *   Reload  = 1249  → timeout   = 1250 / 125 = 10.0 s
 *
 * watchdog_disable() clears the software feed-gate.  The IWDG hardware
 * keeps running; the MCU will reset ~10 s after the last feed unless
 * watchdog_enable() is called again.
 */

/**
 * @brief  Initialise and start the IWDG with a 10-second timeout.
 *
 * Safe to call multiple times; subsequent calls simply refresh the counter
 * and re-arm the software gate.
 */
void watchdog_enable(void);

/**
 * @brief  Clear the software feed-gate.
 *
 * watchdog_feed() becomes a no-op after this call.  Because the IWDG
 * cannot be stopped by hardware, the MCU will reset approximately 10 s
 * after the last successful feed unless watchdog_enable() is called again.
 */
void watchdog_disable(void);

/**
 * @brief  Feed (refresh) the IWDG counter.
 *
 * Must be called more often than every 10 seconds while the watchdog is
 * enabled.  This function is a no-op when the software gate is cleared.
 */
void watchdog_feed(void);

/**
 * @brief  Query the software feed-gate state.
 * @retval Non-zero when the watchdog is enabled (feeding is active).
 */
int watchdog_is_enabled(void);

#ifdef __cplusplus
}
#endif

#endif /* __WATCHDOG_H__ */
