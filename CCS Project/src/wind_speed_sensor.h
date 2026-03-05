#ifndef WIND_SPEED_SENSOR_H
#define WIND_SPEED_SENSOR_H

void   WindSpeed_Init(void);

double WindSpeed_GetHz(void);
double WindSpeed_GetRPM(void);
double WindSpeed_GetMS(void);

#endif