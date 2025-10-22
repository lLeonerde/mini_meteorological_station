#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "freertos/queue.h"
#include "esp_wifi.h"


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
#include "sensor_task.h"
#include "button_task.h"
#include "cloud_task.h"
#define MAX_WIFI_RETRY 2

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


LGFX_OLED_I2C_128x64 lcd;
LGFX_Sprite canvas(&lcd);
my_wifi_config_t my_wifi_config;
QueueHandle_t display_queue;
QueueHandle_t spp_data_queue;
SemaphoreHandle_t button_press_sem;
SemaphoreHandle_t sensor_data_mutex;
sensor_data_t latest_sensor_data;
extern "C"
{
    void main_task(void *param){
        display_message_t display_msg;
        display_msg.state = DISPLAY_STATE_BOOTING;
        xQueueSend(display_queue, &display_msg, 0);
        xEventGroupSetBits(app_event_group, BOOT_CONFIG_TIME_INIT_BIT);
        vTaskDelay(pdMS_TO_TICKS(5000));
        xEventGroupSetBits(app_event_group, BOOT_CONFIG_TIME_END_BIT);
        EventBits_t startup_bits = xEventGroupGetBits(app_event_group);
        if(startup_bits & FORCE_BT_CONFIG_BIT){
            display_msg.state = DISPLAY_STATE_BT_WAITING;
            xQueueSend(display_queue, &display_msg, 0);
            init_bluetooth();
        }else{
            if(get_saved_config(my_nvs_handle, &my_wifi_config)) {
                display_msg.state = DISPLAY_STATE_WIFI_CONNECTING;
                xQueueSend(display_queue, &display_msg, 0);
                printf("DEBUG: Parser extraiu -> SSID: [%s], Senha: [%s]\n", my_wifi_config.ssid, my_wifi_config.password); // <<< ADICIONE AQUI
                // wifi_start
                wifi_connect(my_wifi_config);
            } else {
                display_msg.state = DISPLAY_STATE_BT_WAITING;
                xQueueSend(display_queue, &display_msg, 0);

                // BL start
                init_bluetooth();
            }
        }
       uint8_t retry_count = 0;
       for(;;){
            EventBits_t bits = xEventGroupWaitBits(app_event_group,
                                               WIFI_CONFIG_UPDATED_BIT | WIFI_DISCONNECTED_BIT,
                                               pdTRUE, pdFALSE, portMAX_DELAY);
            //manage connection/reconnections and so
            if(bits & WIFI_CONFIG_UPDATED_BIT) {
                printf("MANAGER: Nova config recebida! Reiniciando WiFi...\n");
                deinit_bluetooth();
                display_msg.state = DISPLAY_STATE_WIFI_CONNECTING;
                xQueueSend(display_queue, &display_msg, 0);
                get_saved_config(my_nvs_handle, &my_wifi_config);
                wifi_connect(my_wifi_config);
            }
            if(bits & WIFI_DISCONNECTED_BIT){
                display_msg.state = DISPLAY_STATE_WIFI_CONNECTING;
                xQueueSend(display_queue, &display_msg, 0);
                if(++retry_count >= MAX_WIFI_RETRY){
                    retry_count = 0;
                    init_bluetooth();
                    display_msg.state = DISPLAY_STATE_BT_WAITING;
                    xQueueSend(display_queue, &display_msg, 0);
                    xEventGroupClearBits(app_event_group, WIFI_DISCONNECTED_BIT);
                }else{
                    printf("retry wifi\n");
                    esp_wifi_connect();
                }
            }
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
        
        display_queue = xQueueCreate(3, sizeof(display_message_t));
        spp_data_queue = xQueueCreate(10, sizeof(spp_data_packet_t));
        button_press_sem = xSemaphoreCreateBinary();
        sensor_data_mutex = xSemaphoreCreateMutex();
        app_event_group = xEventGroupCreate();
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        wifi_init();
        button_init();
        xTaskCreate(command_process_task, "command_process_task", 4096, NULL, 10, NULL);
        xTaskCreate(display_task,"display_task",4096,NULL,9,NULL);
        xTaskCreate(main_task,"main_task",4096,NULL,4,NULL);
        xTaskCreate(sensor_task,"sensor_task",4096,NULL,5,NULL);
        xTaskCreate(button_task,"button_task",4096,NULL,7,NULL);
        xTaskCreate(cloud_task,"cloud_task",4096,NULL,7,NULL);
        while(1){
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
    }
}