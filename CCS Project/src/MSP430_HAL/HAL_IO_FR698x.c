/* --COPYRIGHT--,BSD
 * Copyright (c) 2019, Texas Instruments Incorporated
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
//*****************************************************************************
//   HAL IO functions
//
// Texas Instruments, Inc.
// *****************************************************************************
#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include <HAL.h>
#include "HAL_Config_Private.h"

// Simple callbacks to functions in main
extern void ButtonCallback_SW1(void);
extern void ButtonCallback_SW2(void);


/**** Functions **************************************************************/
void HAL_IO_Init(void)
{

    // Port output low to save power consumption
    // P1.0 = LED1, Output Low
    // P1.1 = SW1, Input pull-up
    // P1.2 = SW2, Input pull-up
    // P1.3 = Unused, Output Low
    // P1.4 = Unused, Output Low
    // P1.5 = Unused, Output Low
    // P1.6 = Unused, Output Low
    // P1.7 = Unused, Output Low
    P1OUT = (BIT1 | BIT2);
    P1DIR = (BIT0 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    P1REN |= (BIT1 | BIT2);
    
    // P2.0 = Unused, Output Low
    // P2.1 = Unused, Output Low
    // P2.2 = Unused, Output Low
    // P2.3 = Unused, Output Low
    // P2.4 = Unused, Output Low
    // P2.5 = Unused, Output Low
    // P2.6 = Unused, Output Low
    // P2.7 = Unused, Output Low
    P2OUT = (0x00);
    P2DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    
    // P3.0 = Unused, Output Low
    // P3.1 = Unused, Output Low
    // P3.2 = Unused, Output Low
    // P3.3 = Unused, Output Low
    // P3.4 = UART TX
    // P3.5 = UART RX
    // P3.6 = Unused, Output Low
    // P3.7 = Unused, Output Low
    P3OUT = (0x00);
    P3DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT6 | BIT7);
    P3SEL0 |= (BIT4 | BIT5);
    P3SEL1 &= ~(BIT4 | BIT5);

    // P4.0 = Unused, Output Low
    // P4.1 = Unused, Output Low
    // P4.2 = Unused, Output Low
    // P4.3 = Unused, Output Low
    // P4.4 = Unused, Output Low
    // P4.5 = Unused, Output Low
    // P4.6 = Unused, Output Low
    // P4.7 = Unused, Output Low
    P4OUT = (BIT5);
    P4DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    
    // P5.0 = Unused, Output Low
    // P5.1 = Unused, Output Low
    // P5.2 = Unused, Output Low
    // P5.3 = Unused, Output Low
    // P5.4 = Unused, Output Low
    // P5.5 = Unused, Output Low
    // P5.6 = Unused, Output Low
    // P5.7 = Unused, Output Low
    P5OUT = (0x00);
    P5DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    
    // P6.0 = Unused, Output Low
    // P6.1 = Unused, Output Low
    // P6.2 = Unused, Output Low
    // P6.3 = Unused, Output Low
    // P6.4 = Unused, Output Low
    // P6.5 = Unused, Output Low
    // P6.6 = Unused, Output Low
    // P6.7 = Unused, Output Low
    P6OUT = (0x00);
    P6DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    
    // P7.0 = Unused, Output Low
    // P7.1 = Unused, Output Low
    // P7.2 = Unused, Output Low
    // P7.3 = Unused, Output Low
    // P7.4 = Unused, Output Low
    // P7.5 = Unused, Output Low
    // P7.6 = Unused, Output Low
    // P7.7 = Unused, Output Low
    P7OUT = (0x00);
    P7DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    
    // P8.0 = Unused, Output Low
    // P8.1 = Unused, Output Low
    // P8.2 = Unused, Output Low
    // P8.3 = Unused, Output Low
    // P8.4 = Unused, Output Low
    // P8.5 = Unused, Output Low
    // P8.6 = Unused, Output Low
    // P8.7 = Unused, Output Low
    P8OUT = (0x00);
    P8DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    
    // P9.0 = Unused, Output Low
    // P9.1 = Unused, Output Low
    // P9.2 = Unused, Output Low
    // P9.3 = Unused, Output Low
    // P9.4 = Unused, Output Low
    // P9.5 = Unused, Output Low
    // P9.6 = Unused, Output Low
    // P9.7 = LED2, Output Low
    P9OUT = (0x00);
    P9DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    
    // P10.0 = Unused, Output Low
    // P10.1 = Unused, Output Low
    // P10.2 = Unused, Output Low
    // P10.3 = Unused, Output Low
    // P10.4 = Unused, Output Low
    // P10.5 = Unused, Output Low
    // P10.6 = Unused, Output Low
    // P10.7 = LED2, Output Low
    P10OUT = (0x00);
    P10DIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);

    // PJ.0 = Unused, Output Low
    // PJ.1 = Unused, Output Low
    // PJ.2 = Unused, Output Low
    // PJ.3 = Unused, Output Low
    // PJ.4 = Unused, Output Low
    // PJ.5 = Unused, Output Low
    // PJ.6 = Unused, Output Low
    // PJ.7 = Unused, Output Low
    PJOUT = (0x00);
    PJDIR = (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);

    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                           // to activate previously configured port settings

}

void HAL_IO_InitButtons(void)
{
     // Configure SW1 and SW2 for interrupts (pins set as input-pullup during GPIO initialization)
    P1IES = (BIT1 | BIT2);                 // Hi/Low edge
    P1IFG = 0;                             // Clear flags
    P1IE = (BIT1 | BIT2);                  // interrupt enabled
}
// Port interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT1_VECTOR))) Port_1 (void)
#else
#error Compiler not supported!
#endif
{
    if (P1IFG & BIT1)
    {
        ButtonCallback_SW1();
        P1IFG &= ~BIT1;                         // Clear IFG
    }
    if (P1IFG & BIT2)
    {
        ButtonCallback_SW2();
        P1IFG &= ~BIT2;                         // Clear IFG
    }
    __bic_SR_register_on_exit(LPM3_bits);   // Exit LPM3
}


