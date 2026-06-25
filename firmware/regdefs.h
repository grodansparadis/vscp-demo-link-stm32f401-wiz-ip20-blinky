// FILE: regdefs.h

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

#ifndef __REGDEFS_H__
#define __REGDEFS_H__

// User registers ordinals
#define REG_DEVICE_ZONE               0 // Zone for the device [P]
#define REG_DEVICE_SUBZONE            1 // Subzone for the device [P]
#define REG_DEVICE_STATUS             2 // Status of the device [P]
#define REG_DEVICE_CONTROL            3 // Control register for the device [P]
#define REG_DEVICE_BLINK_INTERVAL_MSB 4 // Most significant byte of the blink interval [P]
#define REG_DEVICE_BLINK_INTERVAL_LSB 5 // Least significant byte of the blink interval [P]
#define REG_DEVICE_COUNTER_0          6 // Counter 0 
#define REG_DEVICE_COUNTER_1          7 // Counter 1 
#define REG_DEVICE_COUNTER_2          8 // Counter 2 
#define REG_DEVICE_COUNTER_3          9 // Counter 3 
#define REG_DEVICE_BUTTON_BYTE0       10 // Button byte 0 [P]
#define REG_DEVICE_BUTTON_ZONE        11 // Button zone [P]
#define REG_DEVICE_BUTTON_SUBZONE     12 // Button subzone [P]

#endif