#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "HAL.h"
#include "wind_speed_sensor.h"
#include "hal_LCD.h"

/*
 * =========================================================
 * Hardware pin definitions
 * =========================================================
 * LED_RED   -> On-board red LED   (P1.0)
 * LED_GREEN -> On-board green LED (P9.7)
 * S1_PIN    -> User button S1     (P1.1)
 * S2_PIN    -> User button S2     (P1.2)
 */
#define LED_RED     BIT0
#define LED_GREEN   BIT7
#define S1_PIN      BIT1
#define S2_PIN      BIT2

/*
 * Timing constants
 * HOLD_MS    -> Time both buttons must be held to toggle display mode
 * REFRESH_MS -> LCD refresh interval
 */
#define HOLD_MS     800
#define REFRESH_MS  50

/*
 * =========================================================
 * Test mode switch
 * =========================================================
 * TEST_MODE = 1:
 *   Use simulated sine-wave values for RPM and wind speed.
 *
 * TEST_MODE = 0:
 *   Use real measurements from the anemometer.
 */
#define TEST_MODE 0

/* Simulated signal parameters */
#define TEST_PERIOD_SEC 5.0
#define TEST_RPM_AMP    2000.0   // 0 to 2000 to 0
#define TEST_MS_AMP     100.0    // 0 to 100 to 0

/*
 * Approximate loop time step used to advance the simulated sine wave.
 * Assumes __delay_cycles(8000) is about 1 ms at ~8 MHz.
 */
#define LOOP_DT_SEC     0.001

/*
 * Display modes:
 * MODE_RPM -> Show RPM on the LCD
 * MODE_MS  -> Show wind speed in m/s on the LCD
 */
typedef enum { MODE_RPM = 0, MODE_MS = 1 } display_mode_t;
static display_mode_t g_mode = MODE_RPM;

/*
 * Buttons use internal pull-up resistors:
 * pressed   -> logic 0
 * released  -> logic 1
 */
static inline bool S1_Pressed(void) { return (P1IN & S1_PIN) == 0; }
static inline bool S2_Pressed(void) { return (P1IN & S2_PIN) == 0; }

/* Configure LED pins as outputs and start with both LEDs turned off */
static void leds_init(void)
{
    P1DIR |= LED_RED;
    P1OUT &= ~LED_RED;

    P9DIR |= LED_GREEN;
    P9OUT &= ~LED_GREEN;
}

/* Configure S1 and S2 as inputs with internal pull-up resistors enabled */
static void buttons_init(void)
{
    P1DIR &= ~(S1_PIN | S2_PIN);
    P1REN |=  (S1_PIN | S2_PIN);
    P1OUT |=  (S1_PIN | S2_PIN);
}

/* Display exactly 6 characters on the segmented LCD */
static void lcd_show6(const char s6[6])
{
    showChar(s6[0], pos1);
    showChar(s6[1], pos2);
    showChar(s6[2], pos3);
    showChar(s6[3], pos4);
    showChar(s6[4], pos5);
    showChar(s6[5], pos6);
}

/*
 * Format RPM for the 6-character LCD as:
 * "RPM###"
 */
static void show_rpm(double rpm)
{
    uint16_t r = 0;
    if (rpm > 0) r = (uint16_t)(rpm + 0.5);

    if (r > 999) r = 999;

    char s6[6];
    s6[0] = 'R'; s6[1] = 'P'; s6[2] = 'M';

    s6[5] = (char)('0' + (r % 10)); r /= 10;
    s6[4] = (char)('0' + (r % 10)); r /= 10;
    s6[3] = (char)('0' + (r % 10));

    lcd_show6(s6);
}

/*
 * Format wind speed for the 6-character LCD as an integer value.
 */
static void show_ws(double ms)
{
    if (ms < 0) ms = 0;

    uint16_t v = (uint16_t)(ms + 0.5);
    if (v > 999) v = 999;

    char s6[6];

    s6[0] = 'W';
    s6[1] = 'S';

    if (v < 10)
    {
        s6[2] = ' ';
        s6[3] = ' ';
        s6[4] = ' ';
        s6[5] = (char)('0' + v);
    }
    else if (v < 100)
    {
        s6[2] = ' ';
        s6[3] = ' ';
        s6[4] = (char)('0' + (v / 10));
        s6[5] = (char)('0' + (v % 10));
    }
    else
    {
        s6[2] = ' ';
        s6[3] = (char)('0' + (v / 100));
        s6[4] = (char)('0' + ((v / 10) % 10));
        s6[5] = (char)('0' + (v % 10));
    }

    lcd_show6(s6);
}

/*
 * Keep the phase value bounded so it does not grow indefinitely.
 */
static double wrap_phase(double x)
{
    if (x > 1e6) x = fmod(x, 2.0 * 3.141592653589793);
    return x;
}

/*
 * Generate simulated RPM and wind speed signals.
 * Output shape:
 *   RPM: 0 -> 2000 -> 0
 *   m/s: 0 -> 100  -> 0
 */
static void get_test_signals(double *rpm_out, double *ms_out)
{
    static double phase = 0.0;

    const double w = (2.0 * 3.141592653589793) / TEST_PERIOD_SEC;
    phase = wrap_phase(phase + w * LOOP_DT_SEC);

    /* sine_val goes from -1 to +1 */
    const double sine_val = sin(phase);

    /* shift and scale so output goes from 0 to max to 0 */
    *rpm_out = TEST_RPM_AMP * 0.5 * (1.0 + sine_val);
    *ms_out  = TEST_MS_AMP  * 0.5 * (1.0 + sine_val);
}

static void red_blink_service(bool trigger)
{
    static uint16_t on_ms = 0;

    if (trigger && on_ms == 0)
    {
        P1OUT |= LED_RED;
        on_ms = 100;
    }

    if (on_ms)
    {
        on_ms--;
        if (on_ms == 0)
        {
            P1OUT &= ~LED_RED;
        }
    }
}

static void show_label_rpm(void) { lcd_show6("RPM   "); }
static void show_label_ws(void)  { lcd_show6("WS    "); }

static void show_signed_1dec(double x)
{
    if (x > 99.9) x = 99.9;
    if (x < -99.9) x = -99.9;

    int neg = (x < 0);
    if (neg) x = -x;

    uint16_t v = (uint16_t)(x * 10.0 + 0.5);
    uint16_t ip = v / 10;
    uint16_t fp = v % 10;

    char s6[6];
    s6[0] = neg ? '-' : ' ';
    s6[1] = (ip >= 10) ? (char)('0' + (ip / 10)) : ' ';
    s6[2] = (char)('0' + (ip % 10));
    s6[3] = ' ';
    s6[4] = (char)('0' + fp);
    s6[5] = ' ';

    lcd_show6(s6);
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    HAL_System_Init();

    /*
     * On MSP430 FRAM devices, GPIO pins start locked after reset.
     * This unlocks GPIO and LCD functionality.
     */
    PM5CTL0 &= ~LOCKLPM5;

    leds_init();
    buttons_init();

    Init_LCD();
    clearLCD();
    lcd_show6("READY ");
    __delay_cycles(8000000);
    clearLCD();

    WindSpeed_Init();

    __bis_SR_register(GIE);

    uint16_t both_hold_ms = 0;
    bool toggled_this_hold = false;
    uint16_t refresh_ms = 0;

    while (1)
    {
        double rpm, ms;

        __delay_cycles(8000);   // about 1 ms

        /* Button-hold logic */
        if (S1_Pressed() && S2_Pressed())
        {
            if (both_hold_ms < HOLD_MS)
            {
                both_hold_ms++;
            }

            if ((both_hold_ms >= HOLD_MS) && !toggled_this_hold)
            {
                g_mode = (g_mode == MODE_RPM) ? MODE_MS : MODE_RPM;
                toggled_this_hold = true;

                if (g_mode == MODE_RPM) show_label_rpm();
                else                    show_label_ws();
            }
        }
        else
        {
            both_hold_ms = 0;
            toggled_this_hold = false;
        }

        /* Green LED on when any button is pressed */
        if (S1_Pressed() || S2_Pressed()) P9OUT |= LED_GREEN;
        else                              P9OUT &= ~LED_GREEN;

        /* Periodic LCD refresh */
        refresh_ms++;
        if (refresh_ms >= REFRESH_MS)
        {
            refresh_ms = 0;

#if TEST_MODE
            get_test_signals(&rpm, &ms);
#else
            rpm = WindSpeed_GetRPM();
            ms  = WindSpeed_GetMS();
#endif

            if (g_mode == MODE_RPM) show_rpm(rpm);
            else                    show_ws(ms);
        }

        /* Optional blink service */
        red_blink_service(false);
    }
}

/* Required callback symbols expected by the HAL button framework */
void ButtonCallback_SW1(void) { }
void ButtonCallback_SW2(void) { }