#include <msp430.h>
#include <stdint.h>
#include <math.h>
#include "wind_speed_sensor.h"

/*
 * =========================================================
 * Wind speed sensor driver for MSP430FR6989
 * =========================================================
 *
 * Measurement method:
 * - Timer_A0 CCR1 is used in capture mode on rising edges
 * - The last 5 capture timestamps are stored
 * - 4 time differences are computed between consecutive captures
 * - The average period is converted into frequency
 *
 * Conversions:
 * - Wind speed (m/s) = 0.098 * frequency(Hz)
 * - RPM = (frequency / pulses_per_revolution) * 60
 *
 * This matches the same general method used in the MSP432 version.
 */

/* Maximum timer count used by Timer_A */
#define TIMER_PERIOD                    65535u

/* Number of captured timestamps stored for averaging */
#define CAPTURE_VALUES_BUFFER_CAPACITY  5u

/*
 * Timer clock frequency after divider.
 *
 * IMPORTANT:
 * This must match the actual SMCLK configuration.
 *
 * Example:
 * - If SMCLK = 8 MHz and total divider = 32, then tick rate = 250000 Hz
 * - If SMCLK = 3 MHz and total divider = 32, then tick rate = 93750 Hz
 */
#define TIMER_TICK_HZ                   250000.0

/* Sensor-specific conversion constants */
#define HZ_TO_MS                        0.098
#define PULSES_PER_REV                  3.0   // Example: Young 05103 style sensor

/*
 * Capture input pin configuration.
 *
 * The wind sensor pulse output must be connected to the timer capture pin
 * associated with Timer_A0 CCR1.
 *
 * Current mapping here assumes P2.4.
 * This must match the board wiring and device pin function mapping.
 */
#define WS_CAPTURE_PIN      BIT4
#define WS_CAPTURE_DIR      P2DIR
#define WS_CAPTURE_SEL0     P2SEL0
#define WS_CAPTURE_SEL1     P2SEL1
#define WS_CAPTURE_REN      P2REN

/*
 * captureValues[]
 * Stores the most recent timer capture values.
 * captureValues[0] is the newest capture.
 */
static volatile uint16_t captureValues[CAPTURE_VALUES_BUFFER_CAPACITY];

/*
 * queueSize
 * Counts how many captures have been collected so far.
 *
 * threshold
 * Becomes nonzero once enough captures exist to begin valid calculations.
 */
static volatile uint8_t  queueSize = 0;
static volatile uint16_t threshold = 0;

/* Latest computed sensor values */
static volatile double g_hz  = 0.0;
static volatile double g_rpm = 0.0;
static volatile double g_ms  = 0.0;

/*
 * Compute absolute difference between two 16-bit timer values.
 *
 * This is used to estimate the number of timer ticks between captures.
 */
static uint16_t abs_diff_u16(uint16_t a, uint16_t b)
{
    return (a >= b) ? (uint16_t)(a - b) : (uint16_t)(b - a);
}

/*
 * Recompute frequency, RPM, and wind speed using the current capture buffer.
 *
 * Method:
 * - Compute 4 time differences from the 5 stored capture timestamps
 * - Convert each difference from ticks to seconds
 * - Sum the time intervals
 * - Frequency = number_of_intervals / total_time
 *
 * Because 5 captures create 4 intervals, the frequency is:
 *      4 / total_elapsed_time
 */
static void recompute_from_queue(void)
{
    double sum_dt_sec;
    uint8_t i;

    /* Do not compute until enough captures have been collected */
    if (!threshold) return;

    sum_dt_sec = 0.0;

    for (i = 0; i < (CAPTURE_VALUES_BUFFER_CAPACITY - 1); i++)
    {
        uint16_t diff_ticks = abs_diff_u16(captureValues[i], captureValues[i + 1]);
        double dt_sec = ((double)diff_ticks) / TIMER_TICK_HZ;
        sum_dt_sec += dt_sec;
    }

    /* Avoid division by zero */
    if (sum_dt_sec <= 0.0) return;

    /* Frequency in Hz = intervals / total elapsed time */
    g_hz  = (double)(CAPTURE_VALUES_BUFFER_CAPACITY - 1) / sum_dt_sec;

    /* Convert frequency into wind speed and RPM */
    g_ms  = g_hz * HZ_TO_MS;
    g_rpm = (g_hz / PULSES_PER_REV) * 60.0;
}

/*
 * Initialize timer capture hardware and reset measurement state.
 */
void WindSpeed_Init(void)
{
    uint8_t i;

    /* Configure capture pin as peripheral input */
    WS_CAPTURE_DIR &= ~WS_CAPTURE_PIN;
    WS_CAPTURE_REN &= ~WS_CAPTURE_PIN;

    /*
     * Select alternate timer function on P2.4.
     * Exact function depends on MSP430 pin mapping.
     */
    WS_CAPTURE_SEL0 |= WS_CAPTURE_PIN;
    WS_CAPTURE_SEL1 &= ~WS_CAPTURE_PIN;

    /*
     * Timer_A0 configuration:
     * - Source: SMCLK
     * - Divider: ID__8 plus expansion divider
     * - Total divider intended for measurement timing
     * - Period set to maximum 16-bit count
     */
    TA0CTL  = 0;
    TA0EX0  = TAIDEX_3;        // Expansion divider
    TA0CCR0 = TIMER_PERIOD;

    /*
     * CCR1 capture configuration:
     * - Rising edge capture
     * - Capture input CCI1A
     * - Synchronized capture
     * - Capture mode enabled
     * - Interrupt enabled
     */
    TA0CCTL1 = CM_1 | CCIS_0 | SCS | CAP | CCIE;

    /*
     * Start Timer_A0:
     * - Clock source = SMCLK
     * - Input divider = /8
     * - Mode = up/down
     * - Clear timer
     */
    TA0CTL = TASSEL__SMCLK | ID__8 | MC__UPDOWN | TACLR;

    /* Clear capture history */
    for (i = 0; i < CAPTURE_VALUES_BUFFER_CAPACITY; i++)
        captureValues[i] = 0;

    /* Reset runtime state */
    queueSize = 0;
    threshold = 0;

    g_hz = 0.0;
    g_ms = 0.0;
    g_rpm = 0.0;
}

/* Getter functions used by the rest of the project */
double WindSpeed_GetHz(void)  { return g_hz;  }
double WindSpeed_GetRPM(void) { return g_rpm; }
double WindSpeed_GetMS(void)  { return g_ms;  }

/*
 * Timer_A0 interrupt service routine
 *
 * This ISR handles capture events from CCR1.
 * Each time a rising edge is captured:
 * - the newest timestamp is inserted at the front of the buffer
 * - older timestamps are shifted right
 * - once enough captures exist, the sensor values are recomputed
 * - the CPU is woken up from low-power mode
 */
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void)
{
    switch (__even_in_range(TA0IV, TA0IV_TAIFG))
    {
        case TA0IV_NONE:
            break;

        case TA0IV_TACCR1:
        {
            int j;

            /* Shift older capture values to make room for the newest one */
            for (j = (int)CAPTURE_VALUES_BUFFER_CAPACITY - 1; j > 0; j--)
                captureValues[j] = captureValues[j - 1];

            /* Store newest captured timer value */
            captureValues[0] = TA0CCR1;

            /*
             * queueSize tracks buffer fill progress.
             * Once enough captures are available, threshold becomes nonzero,
             * allowing valid recomputation.
             */
            if (queueSize == (CAPTURE_VALUES_BUFFER_CAPACITY - 1))
                threshold++;
            else
                queueSize++;

            /* Recompute only after the buffer has enough valid captures */
            if (threshold)
                recompute_from_queue();

            /* Wake CPU if main loop entered low-power mode */
            __bic_SR_register_on_exit(LPM3_bits);
        }
        break;

        case TA0IV_TACCR2:
            break;

        case TA0IV_TAIFG:
            break;

        default:
            break;
    }
}