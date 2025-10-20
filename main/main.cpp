#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"

#include <LovyanGFX.hpp>
#include <bmp280.h>
#include "wifi_setup.h"
#include "tagoIO_communication.h"
#include "wifi_structure.h"
#include "wifi_data_nvs.h"
#include "wifi_setup.h"

#define LGFX_USE_V1

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
bmp280_t sensor_dev;
nvs_handle_t my_nvs_handle;
my_wifi_config_t my_wifi_config;
extern "C"
{
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

        bool is_bme280 = (sensor_dev.id == BME280_CHIP_ID);
        printf("Sensor encontrado: %s\n", is_bme280 ? "BME280" : "BMP280");

        //init wifi
        if(get_saved_config(my_nvs_handle,&my_wifi_config)){
            //use saved config
            wifi_init_sta(my_wifi_config);
        }else{
            //iniciar bluetooth
        }

        lcd.init();
        canvas.setColorDepth(1);
        canvas.createSprite(128, 64);

        float pressure, temperature, humidity;

        while (1){
            if (bmp280_read_float(&sensor_dev, &temperature, &pressure, &humidity) != ESP_OK){
                printf("Leitura do sensor falhou\n");
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }

            canvas.fillScreen(TFT_BLACK); // clear screen
            canvas.setTextColor(TFT_WHITE);

            // draw new data
            canvas.setTextSize(2);
            canvas.setCursor(5, 5);
            canvas.printf("%.1f C", temperature);
            canvas.drawCircle(88, 7, 2, TFT_WHITE); // °C symbol

            // draw pressure
            canvas.setTextSize(1);
            canvas.setCursor(5, 30);
            canvas.printf("Press: %.0f hPa", pressure / 100.0f); // convert Pa to hPa

            //send data to TagoIO
            esp_err_t err = send_tagoIO_data(temperature, pressure);
            if (err == ESP_OK) {
                printf("Dados enviados com sucesso para o TagoIO!\n");
            } else {
                printf("Falha ao enviar dados para o TagoIO: %s\n", esp_err_to_name(err));
            }
            canvas.pushSprite(0, 0);
            
            vTaskDelay(pdMS_TO_TICKS(10000)); //10 seconds update
        }
    }
}