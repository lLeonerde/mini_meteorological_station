#include "display_manager.h"
#include <LovyanGFX.hpp>
#include <cmath>
#include <stdio.h>
#include "sdkconfig.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include "esp_system.h"

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
    bool in_animation = false;
    const TickType_t animation_frame_delay = pdMS_TO_TICKS(20);
    int animation_angle = 0;
    for(;;){
        TickType_t wait_time = in_animation ? animation_frame_delay : portMAX_DELAY;
        if(xQueueReceive(display_queue, &msg, wait_time) == pdTRUE){
            if(msg.state == DISPLAY_STATE_BOOTING){
                in_animation = true;
                animation_angle = 0;
            }else{
                in_animation = false;
                canvas.fillScreen(TFT_BLACK);
                switch(msg.state){
                    case DISPLAY_STATE_SHOW_DATA:
                        canvas.setTextSize(2);
                        canvas.setTextColor(TFT_WHITE, TFT_BLACK);
                        canvas.setCursor(0, 0);
                        canvas.printf("Temp: %.2f C\n", msg.temperature);
                        canvas.printf("Pres: %.2f hPa\n", msg.pressure);
                        canvas.pushSprite(0, 0);
                        //maybe i will need to create a timer to clear the screen after some time
                        break;
                    case DISPLAY_STATE_BT_WAITING:
                        canvas.setTextSize(1);
                        canvas.setTextColor(TFT_WHITE);
                        canvas.setCursor(20, 0);
                        canvas.printf("BT Setup Mode\n");
                        canvas.setCursor(15, 0);
                        canvas.printf("Connect to device:\n");
                        canvas.setCursor(16, 25);
                        canvas.setTextSize(2);
                        canvas.printf("MyDevice\n");
                        canvas.pushSprite(0, 0);
                        break;
                    case DISPLAY_STATE_WIFI_CONNECTING:
                        canvas.setTextSize(1);
                        canvas.setTextColor(TFT_WHITE, TFT_BLACK);
                        canvas.setCursor(0, 0);
                        canvas.printf("Please configure new wifi\n");
                        //get data  from queue
                        canvas.printf("current wifi: %s\n", "My_SSID");
                        canvas.printf("current password: %s\n", "********");
                        canvas.pushSprite(0, 0);
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
            }
        }
    }
}
