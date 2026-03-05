#ifndef WIND_SPEED_SENSOR_H
#define WIND_SPEED_SENSOR_H
//CCS TRASH

#include <stdint.h>

void WindSpeed_Init(void);
double WindSpeed_GetMS(void);
double WindSpeed_GetMPH(void);

#endif