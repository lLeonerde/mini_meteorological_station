#ifndef APP_GLOBALS_H
#define APP_GLOBALS_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "nvs.h"

#define BUTTON_GPIO GPIO_NUM_0
#define WIFI_CONFIG_UPDATED_BIT   (1 << 0)
#define WIFI_CONNECTED_BIT        (1 << 1)
#define WIFI_DISCONNECTED_BIT     (1 << 2)
#define SHOW_DISPLAY_BIT          (1 << 3)
#define BOOT_CONFIG_TIME_INIT_BIT (1 << 4)
#define BOOT_CONFIG_TIME_END_BIT  (1 << 5)
#define FORCE_BT_CONFIG_BIT       (1 << 6)
typedef struct {
    float temperature;
    float pressure;
} sensor_data_t;

extern QueueHandle_t sensor_data_queue;

extern EventGroupHandle_t app_event_group;
extern nvs_handle_t my_nvs_handle;
extern SemaphoreHandle_t button_press_sem;
extern SemaphoreHandle_t sensor_data_mutex;
extern sensor_data_t latest_sensor_data;
#endif