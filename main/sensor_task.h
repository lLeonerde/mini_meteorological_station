#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H
#include <bmp280.h>
extern bmp280_t sensor_dev;
void sensor_task(void *param);
#endif