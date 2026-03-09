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
#define REFRESH_MS  250

/*
 * =========================================================
 * Test mode switch
 * =========================================================
 * TEST_MODE = 1:
 *   Use simulated sine-wave values for RPM and wind speed.
 *   This is useful for testing the LCD and UI without a sensor.
 *
 * TEST_MODE = 0:
 *   Use real measurements from the anemometer.
 */
#define TEST_MODE 1

/* Simulated signal parameters */
#define TEST_PERIOD_SEC 5.0
#define TEST_RPM_AMP    2000.0   // Simulated RPM range: about -2000 to +2000
#define TEST_MS_AMP     100.0    // Simulated wind speed range: about -100 to +100 m/s

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
 *
 * Only the last 3 digits are shown.
 * Negative RPM values are clamped to 0 for display purposes.
 */
static void show_rpm(double rpm)
{
    uint16_t r = 0;
    if (rpm > 0) r = (uint16_t)(rpm + 0.5);

    char s6[6];
    s6[0] = 'R'; s6[1] = 'P'; s6[2] = 'M';

    s6[5] = (char)('0' + (r % 10)); r /= 10;
    s6[4] = (char)('0' + (r % 10)); r /= 10;
    s6[3] = (char)('0' + (r % 10));

    lcd_show6(s6);
}

/*
 * Format wind speed for the 6-character LCD using one decimal digit.
 *
 * Example:
 *   5.6 m/s -> "WS 5 6"
 *
 * Because the LCD is limited, this function avoids using decimal-point
 * segments and instead shows the integer and fractional parts separately.
 *
 * Negative values are clamped to 0.
 * Maximum displayed value is limited to 99.9.
 */
static void show_ws_1dec(double ms)
{
    if (ms < 0) ms = 0;

    uint16_t v = (uint16_t)(ms * 10.0 + 0.5);
    uint16_t ip = v / 10;
    uint16_t fp = v % 10;

    if (ip > 99) { ip = 99; fp = 9; }

    char s6[6];
    s6[0] = 'W';
    s6[1] = 'S';

    s6[2] = (ip >= 10) ? (char)('0' + (ip / 10)) : ' ';
    s6[3] = (char)('0' + (ip % 10));
    s6[4] = ' ';
    s6[5] = (char)('0' + fp);

    lcd_show6(s6);
}

/*
 * Keep the phase value bounded so it does not grow indefinitely
 * during long runs in test mode.
 */
static double wrap_phase(double x)
{
    if (x > 1e6) x = fmod(x, 2.0 * 3.141592653589793);
    return x;
}

/*
 * Generate simulated RPM and wind speed signals using a sine wave.
 * This function is only used when TEST_MODE == 1.
 */
static void get_test_signals(double *rpm_out, double *ms_out)
{
    static double phase = 0.0;

    const double w = (2.0 * 3.141592653589793) / TEST_PERIOD_SEC;
    phase = wrap_phase(phase + w * LOOP_DT_SEC);

    const double s = sin(phase);

    *rpm_out = TEST_RPM_AMP * s;
    *ms_out  = TEST_MS_AMP  * s;
}

int main(void)
{
    /* Stop the watchdog timer to prevent unwanted resets */
    WDTCTL = WDTPW | WDTHOLD;

    /* Perform board/system initialization through the HAL */
    HAL_System_Init();

    /*
     * On MSP430 FRAM devices, GPIO pins start locked after reset.
     * This unlocks GPIO and LCD functionality.
     */
    PM5CTL0 &= ~LOCKLPM5;

    leds_init();
    buttons_init();

    /* Initialize the LCD and show a short startup message */
    Init_LCD();
    clearLCD();
    lcd_show6("READY ");
    __delay_cycles(8000000);   // About 1 second at ~8 MHz
    clearLCD();

    /* Initialize wind-speed measurement module */
    WindSpeed_Init();

    /* Enable global interrupts */
    __bis_SR_register(GIE);

    uint16_t both_hold_ms = 0;      // How long both buttons have been held
    bool toggled_this_hold = false; // Prevent multiple toggles during one hold
    uint16_t refresh_ms = 0;        // LCD refresh timer

    while (1)
    {
        /* Base loop delay: about 1 ms */
        __delay_cycles(8000);

        /* Red LED heartbeat: shows the firmware is running */
        if ((refresh_ms % 100) == 0) P1OUT ^= LED_RED;

        /*
         * If both buttons are held long enough, toggle the display mode.
         * This switches between RPM display and wind-speed display.
         */
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

        /* Turn on the green LED whenever any button is pressed */
        if (S1_Pressed() || S2_Pressed()) P9OUT |= LED_GREEN;
        else                              P9OUT &= ~LED_GREEN;

        /* Refresh the LCD periodically instead of every loop iteration */
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

        /*
         * Enter low-power mode and wait for an interrupt to wake the CPU.
         * The exact wake-up behavior depends on the ISR/HAL configuration.
         */
        __bis_SR_register(LPM3_bits | GIE);
        __no_operation();
    }
}

/* Required callback symbols expected by the HAL button framework */
void ButtonCallback_SW1(void) { }
void ButtonCallback_SW2(void) { }