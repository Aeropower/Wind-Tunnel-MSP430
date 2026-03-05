#include <msp430.h>
#include <stdint.h>
#include "wind_speed_sensor.h"

#define CAPTURE_VALUES_BUFFER_CAPACITY 5
#define TIMER_TICK_HZ 1000000.0   // SMCLK 8MHz / 8 = 1MHz
//CCS TRASH

// Para debug fácil en CCS:
volatile double windSpeedMS  = 0;
volatile double windSpeedMPH = 0;

static volatile uint16_t captures[CAPTURE_VALUES_BUFFER_CAPACITY];
static volatile uint8_t filled = 0;

static double toMS(double f)  { return f * 0.098; }
static double toMPH(double f) { return 2.23694 * toMS(f); }

void WindSpeed_Init(void)
{
    // Reset timer state
    TA0CCTL1 = 0;
    TA0CTL = 0;

    // Timer_A0: SMCLK/8, continuous
    TA0CTL = TASSEL__SMCLK | ID__8 | MC__CONTINUOUS | TACLR;

    // CCR1 capture: rising edge, CCI1A (TA0.1), sync, interrupt enable
    TA0CCTL1 = CM_1 | CCIS_0 | CAP | SCS | CCIE;
}

double WindSpeed_GetMS(void)  { return windSpeedMS; }
double WindSpeed_GetMPH(void) { return windSpeedMPH; }

#pragma vector = TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void)
{
    switch (__even_in_range(TA0IV, TA0IV_TAIFG))
    {
        case TA0IV_TACCR1:
        {
            uint16_t now = TA0CCR1;
            
            int i;
            int k;


            for (i = CAPTURE_VALUES_BUFFER_CAPACITY - 1; i > 0; i--)
                captures[i] = captures[i-1];
            captures[0] = now;

            if (filled < CAPTURE_VALUES_BUFFER_CAPACITY) filled++;

            if (filled == CAPTURE_VALUES_BUFFER_CAPACITY)
            {
                uint32_t sum = 0;
                for (k = 0; k < CAPTURE_VALUES_BUFFER_CAPACITY - 1; k++)
                {
                    uint16_t a = captures[k];
                    uint16_t b = captures[k+1];
                    uint16_t dt = (a >= b) ? (a - b) : (uint16_t)(a + (uint16_t)(65536 - b));

                    // (opcional) ignora dt imposible
                    if (dt == 0) return;

                    sum += dt;
                }

                double avgDt = (double)sum / (CAPTURE_VALUES_BUFFER_CAPACITY - 1);

                // (opcional) filtro mínimo anti-ruido
                if (avgDt > 1)
                {
                    double f = TIMER_TICK_HZ / avgDt; // Hz
                    windSpeedMS  = toMS(f);
                    windSpeedMPH = toMPH(f);
                }
            }

            __bic_SR_register_on_exit(LPM3_bits);
            break;
        }
        default:
            break;
    }
}