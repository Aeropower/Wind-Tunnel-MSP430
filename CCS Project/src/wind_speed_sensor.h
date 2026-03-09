/*
 * wind_speed_sensor.h
 *
 * Header file for the wind speed sensor module.
 *
 * This file declares the public interface used by other parts of the
 * application to interact with the wind speed sensor logic.
 *
 * The implementation is provided in wind_speed_sensor.c.
 */

#ifndef WIND_SPEED_SENSOR_H
#define WIND_SPEED_SENSOR_H

/*
 * WindSpeed_Init
 *
 * Initializes the wind speed measurement system.
 *
 * Responsibilities typically include:
 * - Configuring the GPIO pin used for the sensor signal
 * - Setting up the Timer_A capture module
 * - Enabling interrupts used to measure pulse timing
 */
void WindSpeed_Init(void);


/*
 * WindSpeed_GetHz
 *
 * Returns the measured pulse frequency from the wind sensor.
 *
 * Units:
 *   Hertz (Hz) = pulses per second
 */
double WindSpeed_GetHz(void);


/*
 * WindSpeed_GetRPM
 *
 * Returns the rotational speed of the anemometer.
 *
 * Units:
 *   Revolutions per minute (RPM)
 *
 * The value is derived from the measured pulse frequency
 * and the number of pulses generated per revolution.
 */
double WindSpeed_GetRPM(void);


/*
 * WindSpeed_GetMS
 *
 * Returns the calculated wind speed.
 *
 * Units:
 *   meters per second (m/s)
 *
 * The conversion is based on the calibration constant of
 * the wind sensor model being used.
 */
double WindSpeed_GetMS(void);

#endif