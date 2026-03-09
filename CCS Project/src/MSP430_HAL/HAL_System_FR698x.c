#include <Config_Common.h>
#include "HAL.h"

/*
 * Local function prototype:
 * used only inside this file to configure the clock system.
 */
static void HAL_System_Clock_Init(void);

/*
 * HAL_System_Init
 *
 * Initializes the basic system hardware required by the application.
 * This includes:
 * - GPIO / port initialization
 * - system clock initialization
 *
 * Returns:
 *   0 -> initialization successful
 */
int8_t HAL_System_Init(void)
{
    HAL_IO_Init();             // Configure all GPIO pins
    HAL_System_Clock_Init();   // Configure CPU and peripheral clocks

    return 0;   // OK
}

/*
 * HAL_System_Clock_Init
 *
 * Configures the MSP430 clock system.
 *
 * Supported configuration:
 * - CPU frequency   = 8 MHz
 * - Low-speed bus   = 10 kHz
 *
 * Clock sources after setup:
 * - MCLK  = DCOCLK = 8 MHz
 * - SMCLK = DCOCLK = 8 MHz
 * - ACLK  = VLOCLK
 */
static void HAL_System_Clock_Init(void)
{
#if ( (CPU_FREQ_HZ == 8000000) && (LSBUS_FREQ_HZ == 10000) )

    /* Unlock Clock System registers */
    CSCTL0_H = CSKEY >> 8;

    /* Set DCO (Digitally Controlled Oscillator) to 8 MHz */
    CSCTL1 = DCOFSEL_6;

    /*
     * Select clock sources:
     * - ACLK  = VLOCLK
     * - SMCLK = DCOCLK
     * - MCLK  = DCOCLK
     */
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;

    /*
     * Set clock dividers to 1:
     * - ACLK  /1
     * - SMCLK /1
     * - MCLK  /1
     */
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;

    /* Lock Clock System registers */
    CSCTL0_H = 0;

#else
#error "Clock configuration is not supported. Modify DualRaySmoke_HAL_System.c"
#endif
}