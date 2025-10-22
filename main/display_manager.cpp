#include "display_manager.h"
#include <LovyanGFX.hpp>
#include <cmath>
#include <stdio.h>
#include "sdkconfig.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include "esp_system.h"
#include "wifi_data_nvs.h"
#define LGFX_USE_V1

#define SHOW_SCREEN_BIT (1 << 3)
#define CONFIG_FREERTOS_HZ 100

constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;

extern lgfx::LGFX_Device lcd;
extern LGFX_Sprite canvas;

void display_task(void *param){
    display_message_t msg;
    lcd.init();
    canvas.setColorDepth(1);
    canvas.createSprite(128, 64);

    const TickType_t display_timeout = pdMS_TO_TICKS(30000);   
    bool is_data_screen_active = false;

    bool in_animation = false;
    const TickType_t animation_frame_delay = pdMS_TO_TICKS(20);

    int animation_angle = 0;
    for(;;){
        TickType_t wait_time = portMAX_DELAY;
        if(in_animation){
            wait_time = animation_frame_delay;
        }else if(is_data_screen_active){
            wait_time = display_timeout;
        }
        if(xQueueReceive(display_queue, &msg, wait_time) == pdTRUE){
            if(msg.state == DISPLAY_STATE_BOOTING){
                in_animation = true;
                animation_angle = 0;
            }else{
                in_animation = false;
                canvas.fillScreen(TFT_BLACK);
                switch(msg.state){
                    case DISPLAY_STATE_SHOW_DATA: 
                    {
                        sensor_data_t latest_read = {0}; // Inicializa com zeros
                        if (xSemaphoreTake(sensor_data_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                            latest_read = latest_sensor_data;
                            xSemaphoreGive(sensor_data_mutex);
                        }

                        // --- TELA DE DADOS MELHORADA ---
                        canvas.setTextDatum(top_left); // Alinhamento do texto no topo à esquerda
                        
                        // Temperatura
                        canvas.setTextSize(2);
                        canvas.setCursor(5, 5);
                        canvas.printf("%.1f", latest_read.temperature);
                        canvas.setTextSize(1);
                        canvas.drawCircle(canvas.getCursorX() + 6, 8, 2, TFT_WHITE); // Símbolo de grau °
                        canvas.setCursor(canvas.getCursorX() + 10, 8);
                        canvas.print("C");

                        // Pressão
                        canvas.setTextSize(1);
                        canvas.setCursor(5, 35);
                        canvas.printf("Pressao: %.0f hPa", latest_read.pressure / 100.0f);
                        
                        // (Opcional) Status da Conexão
                        if (xEventGroupGetBits(app_event_group) & WIFI_CONNECTED_BIT) {
                            canvas.setCursor(5, 50);
                            canvas.print("WiFi: Conectado");
                        } else {
                            canvas.setCursor(5, 50);
                            canvas.print("WiFi: Offline");
                        }

                        is_data_screen_active = true; // Arma o timer de 30s
                        break;
                    }
                    case DISPLAY_STATE_BT_WAITING:
                        canvas.setTextSize(1);
                        canvas.setCursor(15, 5);
                        canvas.printf("Modo de Setup BT");
                        canvas.setCursor(0, 25);
                        canvas.printf("Conecte to device:");
                        canvas.setCursor(0, 35);
                        canvas.setTextSize(1);
                        canvas.printf("METEOROLOGICAL_STATIO");
                        break;
                    case DISPLAY_STATE_BT_CONNECTED:
                        canvas.setTextDatum(top_left); // Alinha o texto no centro
                        canvas.setTextSize(1);
                        canvas.setTextColor(TFT_WHITE, TFT_BLACK);
                        canvas.setCursor(0, 0);
                        canvas.printf("Configure new wifi\n");
                        //get data  from queue
                        my_wifi_config_t my_wifi_config;
                        if(get_saved_config(my_nvs_handle,&my_wifi_config)){
                            canvas.printf("current wifi: \n%s\n", my_wifi_config.ssid);
                            canvas.printf("current password: \n%s", my_wifi_config.password);
                        }else{
                            canvas.printf("current wifi: %s\n", "NONE");
                            canvas.printf("current password: %s\n", "NONE");
                        }
                        canvas.pushSprite(0, 0);
                        break;
                    case DISPLAY_STATE_WIFI_CONNECTING:
                        canvas.setTextDatum(middle_center);
                        canvas.setTextSize(1);
                        canvas.drawString("Conectando ao WiFi...", 64, 32);
                        break;
                    default:
                        break;
                }
                canvas.pushSprite(0, 0);

            }
        }else{
            if (in_animation) {
                canvas.fillScreen(TFT_BLACK);
                canvas.setTextSize(1);
                canvas.setCursor(20, 20);
                canvas.print("Inicializando...");
                canvas.drawCircle(64, 45, 10, TFT_WHITE);
                int x = 64 + static_cast<int>(10.0f * cos(animation_angle * DEG_TO_RAD));
                int y = 45 + static_cast<int>(10.0f * sin(animation_angle * DEG_TO_RAD));
                canvas.fillCircle(x, y, 2, TFT_WHITE);
                canvas.pushSprite(0, 0);
                animation_angle = (animation_angle + 15) % 360;
            } else if (is_data_screen_active) {
                printf("DISPLAY: Timeout, apagando a tela.\n");
                canvas.fillScreen(TFT_BLACK);
                canvas.pushSprite(0, 0);
                is_data_screen_active = false; 
            }
        }
    }
}
