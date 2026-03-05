#include <msp430.h>
#include "HAL.h"
#include "wind_speed_sensor.h"

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog

    HAL_System_Init();          // Clock + GPIO
    WindSpeed_Init();           // Timer capture del anemómetro

    __bis_SR_register(GIE);     // Enable global interrupts

    while (1)
    {
        __bis_SR_register(LPM3_bits | GIE);   // Sleep hasta interrupción
        __no_operation();                     // Para debug
    }
}

// Stubs por si HAL usa botones
void ButtonCallback_SW1(void) { }
void ButtonCallback_SW2(void) { }