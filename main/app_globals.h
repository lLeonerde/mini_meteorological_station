#ifndef APP_GLOBALS_H
#define APP_GLOBALS_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "nvs.h"


#define WIFI_CONFIG_UPDATED_BIT  (1 << 0)
#define WIFI_CONNECTED_BIT       (1 << 1)
#define WIFI_DISCONNECTED_BIT    (1 << 2)

typedef struct {
    float temperature;
    float pressure;
} sensor_data_t;

extern QueueHandle_t sensor_data_queue;

extern EventGroupHandle_t app_event_group;
extern nvs_handle_t my_nvs_handle;

#endif