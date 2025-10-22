#include "cloud_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "app_globals.h"
#include "tagoIO_communication.h"
void cloud_task(void *param){
    sensor_data_t latest_read;
    latest_read.pressure = 0;
    latest_read.temperature = 0;
    for(;;){
        vTaskDelay(pdMS_TO_TICKS(30000));
        if (xEventGroupGetBits(app_event_group) & WIFI_CONNECTED_BIT) {
            if (xSemaphoreTake(sensor_data_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                latest_read = latest_sensor_data; // Atualiza o placard
                xSemaphoreGive(sensor_data_mutex); // Devolve a chave
            }
            send_tagoIO_data(latest_read.temperature, latest_read.pressure);
        }
    }
}