#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>


#include "HAL.h"
#include "wind_speed_sensor.h"
#include "hal_LCD.h"

// LEDs
#define LED_RED     BIT0      // P1.0
#define LED_GREEN   BIT7      // P9.7

// Buttons
#define S1_PIN      BIT1      // P1.1
#define S2_PIN      BIT2      // P1.2

#define HOLD_MS     800
#define REFRESH_MS  250


// ===================== TEST MODE SWITCH =====================
// 1 = generate fake sine-wave RPM + wind speed (no sensor needed)
// 0 = use real anemometer capture values
#define TEST_MODE 1
// ============================================================

// Sine period in seconds
#define TEST_PERIOD_SEC 5.0

// Amplitudes
#define TEST_RPM_AMP    2000.0   // -2000..+2000
#define TEST_MS_AMP     100.0    // -100..+100

// If your loop tick is ~1 ms (because __delay_cycles(8000) at ~8MHz)
#define LOOP_DT_SEC     0.001




typedef enum { MODE_RPM = 0, MODE_MS = 1 } display_mode_t;
static display_mode_t g_mode = MODE_RPM;

static inline bool S1_Pressed(void) { return (P1IN & S1_PIN) == 0; }
static inline bool S2_Pressed(void) { return (P1IN & S2_PIN) == 0; }

static void leds_init(void)
{
    P1DIR |= LED_RED;
    P1OUT &= ~LED_RED;

    P9DIR |= LED_GREEN;
    P9OUT &= ~LED_GREEN;
}

static void buttons_init(void)
{
    P1DIR &= ~(S1_PIN | S2_PIN);
    P1REN |=  (S1_PIN | S2_PIN);
    P1OUT |=  (S1_PIN | S2_PIN); // pull-ups
}

static void lcd_show6(const char s6[6])
{
    showChar(s6[0], pos1);
    showChar(s6[1], pos2);
    showChar(s6[2], pos3);
    showChar(s6[3], pos4);
    showChar(s6[4], pos5);
    showChar(s6[5], pos6);
}

static void show_rpm(double rpm)
{
    // show "RPM###" (last 3 digits)
    uint16_t r = 0;
    if (rpm > 0) r = (uint16_t)(rpm + 0.5);

    char s6[6];
    s6[0] = 'R'; s6[1] = 'P'; s6[2] = 'M';

    // last 3 digits
    s6[5] = (char)('0' + (r % 10)); r /= 10;
    s6[4] = (char)('0' + (r % 10)); r /= 10;
    s6[3] = (char)('0' + (r % 10));

    lcd_show6(s6);
}

static void show_ws_1dec(double ms)
{
    // show "WSx y" = 1 decimal because LCD is 6 chars and we avoid '.' segments
    // Example: 5.6 m/s -> "WS 5 6"
    if (ms < 0) ms = 0;

    uint16_t v = (uint16_t)(ms * 10.0 + 0.5); // scaled 1 decimal
    uint16_t ip = v / 10;
    uint16_t fp = v % 10;

    if (ip > 99) { ip = 99; fp = 9; }

    char s6[6];
    s6[0] = 'W';
    s6[1] = 'S';

    // tens of integer
    s6[2] = (ip >= 10) ? (char)('0' + (ip / 10)) : ' ';
    s6[3] = (char)('0' + (ip % 10));

    s6[4] = ' ';               // separator
    s6[5] = (char)('0' + fp);  // decimal digit

    lcd_show6(s6);
}


///////////////////////////

static double wrap_phase(double x)
{
    // keep x in a reasonable range to avoid float blowup over time
    // not strictly needed, but nice.
    if (x > 1e6) x = fmod(x, 2.0 * 3.141592653589793);
    return x;
}

static void get_test_signals(double *rpm_out, double *ms_out)
{
    // phase advances based on LOOP_DT_SEC; period set by TEST_PERIOD_SEC
    static double phase = 0.0;

    const double w = (2.0 * 3.141592653589793) / TEST_PERIOD_SEC;
    phase = wrap_phase(phase + w * LOOP_DT_SEC);

    // sine wave
    const double s = sin(phase);

    *rpm_out = TEST_RPM_AMP * s;
    *ms_out  = TEST_MS_AMP  * s;
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    HAL_System_Init();

    // Critical: unlock GPIO/LCD pins on FRAM parts
    PM5CTL0 &= ~LOCKLPM5;

    leds_init();
    buttons_init();

    Init_LCD();
    clearLCD();
    lcd_show6("READY ");
    __delay_cycles(8000000);   // ~1 second
    clearLCD();

    WindSpeed_Init();

    __bis_SR_register(GIE);

    uint16_t both_hold_ms = 0;
    bool toggled_this_hold = false;
    uint16_t refresh_ms = 0;

    while (1)
    {
        __delay_cycles(8000); // ~1ms if CPU ~8MHz

        // Alive blink
        if ((refresh_ms % 100) == 0) P1OUT ^= LED_RED;

        // hold S1+S2 to toggle
        if (S1_Pressed() && S2_Pressed())
        {
            if (both_hold_ms < 2000) both_hold_ms++;

            if (both_hold_ms >= HOLD_MS && !toggled_this_hold)
            {
                g_mode = (g_mode == MODE_RPM) ? MODE_MS : MODE_RPM;
                toggled_this_hold = true;
                clearLCD();
            }
        }
        else
        {
            both_hold_ms = 0;
            toggled_this_hold = false;
        }

        // green LED when any button pressed
        if (S1_Pressed() || S2_Pressed()) P9OUT |= LED_GREEN;
        else                             P9OUT &= ~LED_GREEN;

        // refresh LCD
        refresh_ms++;
        if (refresh_ms >= REFRESH_MS)
        {
            refresh_ms = 0;

          double rpm, ms;

            #if TEST_MODE
                get_test_signals(&rpm, &ms);
            #else
                rpm = WindSpeed_GetRPM();
                ms  = WindSpeed_GetMS();
            #endif

            if (g_mode == MODE_RPM) show_rpm(rpm);
            else                    show_ws_1dec(ms);
        }

        __bis_SR_register(LPM3_bits | GIE);
        __no_operation();
    }
}

// HAL expects these symbols
void ButtonCallback_SW1(void) { }
void ButtonCallback_SW2(void) { }