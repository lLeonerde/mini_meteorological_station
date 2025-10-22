
#include "button_task.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/semphr.h"
#include "app_globals.h"
#include "driver/gpio.h"
#include "display_manager.h"

static void IRAM_ATTR button_isr_handler(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(button_press_sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}
void button_task(void *param){
    TickType_t last_press_time = 0;
    const TickType_t debounce_delay = pdMS_TO_TICKS(200);

    for(;;){
        if(xSemaphoreTake(button_press_sem,portMAX_DELAY) == pdTRUE){
            TickType_t current_time = xTaskGetTickCount();
            if((current_time - last_press_time) > debounce_delay){
                last_press_time = current_time;
                EventBits_t bits = xEventGroupGetBits(app_event_group);
                if ((bits & BOOT_CONFIG_TIME_INIT_BIT) && !(bits & BOOT_CONFIG_TIME_END_BIT)) {
                    printf("BUTTON_TASK: Pressionado na janela de boot! Forçando modo BT.\n");
                    xEventGroupSetBits(app_event_group, FORCE_BT_CONFIG_BIT);
                } else {
                    printf("BUTTON_TASK: Pressionado em modo normal. Mostrando tela.\a\n");
                    display_message_t msg = {.state = DISPLAY_STATE_SHOW_DATA}; //show last info available
                    xQueueSend(display_queue, &msg, 0);

                }
            }
        }
    }
}

void button_init() {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = (1ULL << BUTTON_GPIO);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL);
}