/* --COPYRIGHT--,BSD
 * Copyright (c) 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

/*******************************************************************************
 *
 * hal_LCD.h
 *
 * Hardware abstraction layer header for the FH-1138P segmented LCD
 * on the MSP-EXP430FR6989 LaunchPad.
 *
 * This file provides:
 * - LCD digit position definitions
 * - External declarations for character lookup tables
 * - Function prototypes for LCD control
 *
 ******************************************************************************/

#ifndef OUTOFBOX_MSP430FR6989_HAL_LCD_H_
#define OUTOFBOX_MSP430FR6989_HAL_LCD_H_

/*
 * =========================================================
 * LCD position mapping
 * =========================================================
 *
 * These macros define the starting LCD memory index for each of the
 * 6 visible character positions on the segmented LCD.
 *
 * The mapping is hardware-specific and depends on how the LCD glass
 * is wired to the MSP430FR6989 LaunchPad.
 *
 * Position meaning:
 * pos1 -> leftmost displayed character
 * pos6 -> rightmost displayed character
 */
#define pos1 9   /* Digit A1 begins at segment S18 */
#define pos2 5   /* Digit A2 begins at segment S10 */
#define pos3 3   /* Digit A3 begins at segment S6  */
#define pos4 18  /* Digit A4 begins at segment S36 */
#define pos5 14  /* Digit A5 begins at segment S28 */
#define pos6 7   /* Digit A6 begins at segment S14 */

/*
 * =========================================================
 * LCD memory word access
 * =========================================================
 *
 * LCDMEM is normally byte-addressed.
 * This macro provides word-based access to LCD memory when needed.
 */
#ifndef LCDMEMW
#define LCDMEMW    ((int*) LCDMEM) /* LCD memory access as words */
#endif

/*
 * =========================================================
 * External shared variables / lookup tables
 * =========================================================
 *
 * mode:
 *   Global display mode variable used by some LCD helper functions
 *   such as scrolling text.
 *
 * digit:
 *   Lookup table used to display numeric digits (0-9).
 *
 * alphabetBig:
 *   Lookup table used to display uppercase letters (A-Z).
 */
extern volatile unsigned char mode;
extern const char digit[10][2];
extern const char alphabetBig[26][2];

/*
 * =========================================================
 * LCD control function prototypes
 * =========================================================
 */

/*
 * Initialize and enable the segmented LCD peripheral.
 */
void Init_LCD(void);

/*
 * Scroll a message across the 6-character LCD display.
 */
void displayScrollText(char*);

/*
 * Display a single character at the specified LCD position.
 */
void showChar(char, int);

/*
 * Clear all visible LCD positions and related memory locations.
 */
void clearLCD(void);

#endif /* OUTOFBOX_MSP430FR6989_HAL_LCD_H_ */