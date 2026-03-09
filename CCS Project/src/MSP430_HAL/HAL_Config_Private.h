/* --COPYRIGHT--,BSD
 * Copyright (c) 2019, Texas Instruments Incorporated
 * All rights reserved.
 */

/******************************************************************************
 *
 * HAL_Config_Private.h
 *
 * Private configuration file for the MSP430 HAL.
 *
 * Purpose of this file:
 * - Define internal configuration parameters for the HAL
 * - Configure UART communication used for GUI/debug communication
 * - Map generic HAL register names to the correct MSP430 UART registers
 *
 * NOTE:
 * This file is intended for internal HAL configuration and normally
 * should not be modified by application code.
 *
 ******************************************************************************/

#ifndef __CONFIG_PRIVATE_H__
#define __CONFIG_PRIVATE_H__

#include "Config_Common.h"

/*
 * =========================================================
 * GUI Communication Configuration
 * =========================================================
 *
 * Defines the UART settings used for communication between
 * the MSP430 and an external GUI or serial interface.
 */

/*
 * UART communication baud rate.
 *
 * Example:
 * 9600 = 9600 bits per second
 */
#define HAL_GUICOMM_BAUDRATE (9600)


/*
 * Select which eUSCI_A module is used for UART communication.
 *
 * MSP430FR6989 provides multiple UART modules:
 *   UCA0
 *   UCA1
 *
 * Setting:
 *   0 → use UCA0
 *   1 → use UCA1
 */
#define GUI_COMM_UART_EUSCI (1)


/*
 * =========================================================
 * UART Register Mapping
 * =========================================================
 *
 * The HAL uses generic register names (UCAnXXXX).
 * These macros map them to the correct hardware registers
 * depending on which UART module is selected.
 *
 * This allows the HAL code to remain portable without
 * hardcoding UCA0 or UCA1 everywhere.
 */

#if (GUI_COMM_UART_EUSCI == 0)

/*
 * Map generic register names to UART module A0 registers
 */

#define UCAnBR0    (UCA0BR0)     /* Baud rate register low byte */
#define UCAnBR1    (UCA0BR1)     /* Baud rate register high byte */
#define UCAnCTLW0  (UCA0CTLW0)   /* Control register */
#define UCAnIE     (UCA0IE)      /* Interrupt enable register */
#define UCAnIFG    (UCA0IFG)     /* Interrupt flag register */
#define UCAnIV     (UCA0IV)      /* Interrupt vector register */
#define UCAnMCTLW  (UCA0MCTLW)   /* Modulation control register */
#define UCAnSTATW  (UCA0STATW)   /* Status register */
#define UCAnTXBUF  (UCA0TXBUF)   /* Transmit buffer */
#define UCAnRXBUF  (UCA0RXBUF)   /* Receive buffer */

/* UART interrupt vector and ISR name */
#define USCI_An_VECTOR (USCI_A0_VECTOR)
#define USCI_An_ISR    (USCI_A0_ISR)


#elif (GUI_COMM_UART_EUSCI == 1)

/*
 * Map generic register names to UART module A1 registers
 */

#define UCAnBR0    (UCA1BR0)     /* Baud rate register low byte */
#define UCAnBR1    (UCA1BR1)     /* Baud rate register high byte */
#define UCAnCTLW0  (UCA1CTLW0)   /* Control register */
#define UCAnIE     (UCA1IE)      /* Interrupt enable register */
#define UCAnIFG    (UCA1IFG)     /* Interrupt flag register */
#define UCAnIV     (UCA1IV)      /* Interrupt vector register */
#define UCAnMCTLW  (UCA1MCTLW)   /* Modulation control register */
#define UCAnSTATW  (UCA1STATW)   /* Status register */
#define UCAnTXBUF  (UCA1TXBUF)   /* Transmit buffer */
#define UCAnRXBUF  (UCA1RXBUF)   /* Receive buffer */

/* UART interrupt vector and ISR name */
#define USCI_An_VECTOR (USCI_A1_VECTOR)
#define USCI_An_ISR    (USCI_A1_ISR)


#else

/*
 * If an invalid UART module number is selected,
 * stop compilation and notify the developer.
 */
#error "eUSCI is not defined. Modify HAL layer"

#endif

#endif //__CONFIG_PRIVATE_H__