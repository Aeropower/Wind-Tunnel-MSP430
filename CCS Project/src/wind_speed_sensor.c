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
 */

/* Maximum timer count used by Timer_A */
#define TIMER_PERIOD                    65535u

/* Number of captured timestamps stored for averaging */
#define CAPTURE_VALUES_BUFFER_CAPACITY  5u

/*
 * Timer clock frequency after divider.
 * With SMCLK = 8 MHz and total divider = 32:
 * TIMER_TICK_HZ = 250000 Hz
 */
#define TIMER_TICK_HZ                   250000.0

/* Sensor-specific conversion constants */
#define HZ_TO_MS                        0.098
#define PULSES_PER_REV                  3.0

/*
 * Capture input pin configuration.
 *
 * TEST VERSION:
 * The wind sensor pulse output is now assumed to be connected to P2.1.
 *
 * IMPORTANT:
 * This will only work if P2.1 is mapped to the Timer_A capture input
 * used by TA0CCR1 / CCI1A on the MSP430FR6989.
 */
#define WS_CAPTURE_PIN      BIT1
#define WS_CAPTURE_DIR      P2DIR
#define WS_CAPTURE_SEL0     P2SEL0
#define WS_CAPTURE_SEL1     P2SEL1
#define WS_CAPTURE_REN      P2REN

static volatile uint16_t captureValues[CAPTURE_VALUES_BUFFER_CAPACITY];
static volatile uint8_t  queueSize = 0;
static volatile uint16_t threshold = 0;

static volatile double g_hz  = 0.0;
static volatile double g_rpm = 0.0;
static volatile double g_ms  = 0.0;

static uint16_t abs_diff_u16(uint16_t a, uint16_t b)
{
    return (a >= b) ? (uint16_t)(a - b) : (uint16_t)(b - a);
}

static void recompute_from_queue(void)
{
    double sum_dt_sec;
    uint8_t i;

    if (!threshold) return;

    sum_dt_sec = 0.0;

    for (i = 0; i < (CAPTURE_VALUES_BUFFER_CAPACITY - 1); i++)
    {
        uint16_t diff_ticks = abs_diff_u16(captureValues[i], captureValues[i + 1]);
        double dt_sec = ((double)diff_ticks) / TIMER_TICK_HZ;
        sum_dt_sec += dt_sec;
    }

    if (sum_dt_sec <= 0.0) return;

    g_hz  = (double)(CAPTURE_VALUES_BUFFER_CAPACITY - 1) / sum_dt_sec;
    g_ms  = g_hz * HZ_TO_MS;
    g_rpm = (g_hz / PULSES_PER_REV) * 60.0;
}

void WindSpeed_Init(void)
{
    uint8_t i;

    /* Configure capture pin as input with internal pull-up enabled */
    WS_CAPTURE_DIR &= ~WS_CAPTURE_PIN;
    WS_CAPTURE_REN |= WS_CAPTURE_PIN;
    P2OUT |= WS_CAPTURE_PIN;

    /*
     * Select alternate timer function on P2.1.
     * This only works if the hardware pin mapping matches TA0CCR1/CCI1A.
     */
    WS_CAPTURE_SEL0 |= WS_CAPTURE_PIN;
    WS_CAPTURE_SEL1 &= ~WS_CAPTURE_PIN;

    /* Timer_A0 configuration */
    TA0CTL  = 0;
    TA0EX0  = TAIDEX_3;        // expansion divider
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

    for (i = 0; i < CAPTURE_VALUES_BUFFER_CAPACITY; i++)
        captureValues[i] = 0;

    queueSize = 0;
    threshold = 0;

    g_hz = 0.0;
    g_ms = 0.0;
    g_rpm = 0.0;
}

double WindSpeed_GetHz(void)  { return g_hz;  }
double WindSpeed_GetRPM(void) { return g_rpm; }
double WindSpeed_GetMS(void)  { return g_ms;  }

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

            P1OUT ^= BIT0;   /* Debug: toggle red LED on every capture event */

            for (j = (int)CAPTURE_VALUES_BUFFER_CAPACITY - 1; j > 0; j--)
                captureValues[j] = captureValues[j - 1];

            captureValues[0] = TA0CCR1;

            if (queueSize == (CAPTURE_VALUES_BUFFER_CAPACITY - 1))
                threshold++;
            else
                queueSize++;

            if (threshold)
                recompute_from_queue();

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