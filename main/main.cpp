#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "freertos/queue.h"


#include <LovyanGFX.hpp>
#include <bmp280.h>
#include "wifi_setup.h"
#include "tagoIO_communication.h"
#include "wifi_structure.h"
#include "wifi_data_nvs.h"
#include "wifi_setup.h"
#include "display_manager.h"
#include "command_process.h"
#include "bluetooth.h"


class LGFX_OLED_I2C_128x64 : public lgfx::LGFX_Device
{
    lgfx::Panel_SSD1306 _panel_instance;
    lgfx::Bus_I2C       _bus_instance;

public:
    LGFX_OLED_I2C_128x64(void)
    {
        {
            auto cfg = _bus_instance.config();
            cfg.i2c_port    = 0;
            cfg.i2c_addr    = 0x3C;
            cfg.pin_sda     = 5;
            cfg.pin_scl     = 4;
            cfg.freq_write  = 400000;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();
            cfg.pin_rst     = -1;
            cfg.panel_width = 128;
            cfg.panel_height = 64;
            _panel_instance.config(cfg);
        }
        setPanel(&_panel_instance);
    }
};

bmp280_t sensor_dev;
LGFX_OLED_I2C_128x64 lcd;
LGFX_Sprite canvas(&lcd);
my_wifi_config_t my_wifi_config;
QueueHandle_t display_queue;
extern "C"
{
    void main_task(void *param){
        display_message_t display_msg;
        display_msg.state = DISPLAY_STATE_BOOTING;
        xQueueSend(display_queue, &display_msg, 0);
        vTaskDelay(pdMS_TO_TICKS(5000));
        if (get_saved_config(my_nvs_handle, &my_wifi_config)) {

            display_msg.state = DISPLAY_STATE_WIFI_CONNECTING;
            xQueueSend(display_queue, &display_msg, 0);

            // wifi_start
            wifi_init_sta(my_wifi_config);
        } else {

            display_msg.state = DISPLAY_STATE_BT_WAITING;
            xQueueSend(display_queue, &display_msg, 0);

            // BL start
            init_bluetooth();
       }
       for(;;){
            vTaskDelay(pdMS_TO_TICKS(2000));
       }
    }

    void app_main(void){
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
            
        }
        ESP_ERROR_CHECK(ret);
        ret = nvs_open("storage", NVS_READWRITE, &my_nvs_handle);
        if(ret != ESP_OK){
            printf("NVS_ERROR\n");
        }
        ESP_ERROR_CHECK(i2cdev_init());
        
        
        bmp280_params_t params;
        bmp280_init_default_params(&params);
        memset(&sensor_dev, 0, sizeof(bmp280_t));
        ESP_ERROR_CHECK(bmp280_init_desc(&sensor_dev, BMP280_I2C_ADDRESS_0, I2C_NUM_0, (gpio_num_t)5, (gpio_num_t)4));
        ESP_ERROR_CHECK(bmp280_init(&sensor_dev, &params));
        display_queue = xQueueCreate(3, sizeof(display_message_t));
        app_event_group = xEventGroupCreate();
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        xTaskCreate(command_process_task, "command_process_task", 2048, NULL, 10, NULL);
        xTaskCreate(display_task,"display_task",2048,NULL,7,NULL);
        xTaskCreate(main_task,"main_task",4096,NULL,7,NULL);

        while (1){
            vTaskDelay(pdMS_TO_TICKS(10000)); //10 seconds update
        }
    }
}