/* --COPYRIGHT--,BSD
 * Copyright (c) 2019, Texas Instruments Incorporated
 * All rights reserved.
 */

/******************************************************************************
 *
 * HAL_IO.c
 *
 * Hardware Abstraction Layer for GPIO and Button handling.
 *
 * Responsibilities of this file:
 * - Configure all GPIO ports of the MSP430FR6989
 * - Set unused pins to a safe low-power state
 * - Configure push buttons SW1 and SW2
 * - Handle button interrupts
 *
 ******************************************************************************/

#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include <HAL.h>
#include "HAL_Config_Private.h"

/*
 * External callback functions defined in main.c.
 * These are called when the buttons generate an interrupt.
 */
extern void ButtonCallback_SW1(void);
extern void ButtonCallback_SW2(void);


/**** Functions **************************************************************/

/*
 * HAL_IO_Init
 *
 * Initializes all I/O ports of the MSP430.
 *
 * Design principle:
 *  - Unused pins → output LOW (reduces power consumption)
 *  - Buttons → input with pull-up resistors
 *  - UART pins → configured for peripheral function
 *  - LEDs → outputs
 */
void HAL_IO_Init(void)
{

    /*
     * =============================
     * PORT 1 CONFIGURATION
     * =============================
     *
     * P1.0 = LED1
     * P1.1 = SW1 button (input pull-up)
     * P1.2 = SW2 button (input pull-up)
     * Other pins unused
     */

    P1OUT = (BIT1 | BIT2);                  // Enable pull-up level for buttons
    P1DIR = (BIT0 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7); // Outputs except button pins
    P1REN |= (BIT1 | BIT2);                 // Enable pull-up resistors for SW1 and SW2
    

    /*
     * =============================
     * PORT 2 CONFIGURATION
     * =============================
     *
     * All pins unused here, set as outputs LOW.
     * (Some modules may later reconfigure pins if needed.)
     */

    P2OUT = (0x00);
    P2DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    

    /*
     * =============================
     * PORT 3 CONFIGURATION
     * =============================
     *
     * P3.4 = UART TX
     * P3.5 = UART RX
     * Other pins unused
     */

    P3OUT = (0x00);
    P3DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT6 | BIT7); // Outputs
    P3SEL0 |= (BIT4 | BIT5);               // Select UART peripheral function
    P3SEL1 &= ~(BIT4 | BIT5);


    /*
     * =============================
     * PORT 4 CONFIGURATION
     * =============================
     *
     * All pins unused → outputs
     */

    P4OUT = (BIT5);                        // Set one pin HIGH (board-specific)
    P4DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    

    /*
     * =============================
     * PORT 5 CONFIGURATION
     * =============================
     */

    P5OUT = (0x00);
    P5DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    

    /*
     * =============================
     * PORT 6 CONFIGURATION
     * =============================
     */

    P6OUT = (0x00);
    P6DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    

    /*
     * =============================
     * PORT 7 CONFIGURATION
     * =============================
     */

    P7OUT = (0x00);
    P7DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    

    /*
     * =============================
     * PORT 8 CONFIGURATION
     * =============================
     */

    P8OUT = (0x00);
    P8DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    

    /*
     * =============================
     * PORT 9 CONFIGURATION
     * =============================
     *
     * P9.7 = LED2 (green LED on LaunchPad)
     */

    P9OUT = (0x00);
    P9DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    

    /*
     * =============================
     * PORT 10 CONFIGURATION
     * =============================
     */

    P10OUT = (0x00);
    P10DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);


    /*
     * =============================
     * PORT J CONFIGURATION
     * =============================
     */

    PJOUT = (0x00);
    PJDIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);


    /*
     * Disable the default high-impedance mode of GPIO pins.
     *
     * After reset, MSP430 FRAM devices keep GPIOs locked
     * in high-impedance mode until this bit is cleared.
     */
    PM5CTL0 &= ~LOCKLPM5;
}


/*
 * HAL_IO_InitButtons
 *
 * Enables interrupts for SW1 and SW2 buttons.
 *
 * Buttons already configured as input pull-ups in HAL_IO_Init().
 */
void HAL_IO_InitButtons(void)
{
     // Interrupt on falling edge (button press)
    P1IES = (BIT1 | BIT2);

    // Clear any pending interrupt flags
    P1IFG = 0;

    // Enable interrupts for SW1 and SW2
    P1IE = (BIT1 | BIT2);
}


/*
 * Port 1 Interrupt Service Routine
 *
 * This ISR executes when either SW1 or SW2 generates an interrupt.
 */
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)

#elif defined(__GNUC__)

void __attribute__ ((interrupt(PORT1_VECTOR))) Port_1 (void)

#else
#error Compiler not supported!
#endif
{

    /*
     * If SW1 caused the interrupt
     */
    if (P1IFG & BIT1)
    {
        ButtonCallback_SW1();   // Call application callback
        P1IFG &= ~BIT1;         // Clear interrupt flag
    }

    /*
     * If SW2 caused the interrupt
     */
    if (P1IFG & BIT2)
    {
        ButtonCallback_SW2();   // Call application callback
        P1IFG &= ~BIT2;         // Clear interrupt flag
    }

    /*
     * Wake the CPU from Low Power Mode 3 (LPM3)
     */
    __bic_SR_register_on_exit(LPM3_bits);
}