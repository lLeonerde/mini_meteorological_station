
#include "sensor_task.h"
#include <string.h>
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "app_globals.h"
#include "display_manager.h"
void sensor_task(void *param){
    bmp280_t sensor_dev;
    bmp280_params_t params;
    bmp280_init_default_params(&params);
    memset(&sensor_dev, 0, sizeof(bmp280_t));
    ESP_ERROR_CHECK(bmp280_init_desc(&sensor_dev, BMP280_I2C_ADDRESS_0, I2C_NUM_0, (gpio_num_t)5, (gpio_num_t)4));
    ESP_ERROR_CHECK(bmp280_init(&sensor_dev, &params));
    sensor_data_t current_reading;
    for(;;){
        float temperature; float pressure; float humidity;
        if (bmp280_read_float(&sensor_dev, &temperature, &pressure, &humidity) == ESP_OK){
            current_reading.temperature = temperature;
            current_reading.pressure = pressure;
            if (xSemaphoreTake(sensor_data_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                latest_sensor_data = current_reading; // Atualiza o placard
                xSemaphoreGive(sensor_data_mutex); // Devolve a chave
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10000)); // 10 seconds delay
    }
}