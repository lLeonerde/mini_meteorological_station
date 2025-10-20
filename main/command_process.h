#ifndef MAIN_COMMAND_PROCESS_H_
#define MAIN_COMMAND_PROCESS_H_
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>

#define SPP_DATA_MAX_LEN 128
typedef enum {
    CMD_UNKNOWN,
    CMD_SET_WIFI_CFG,
} command_type_t;

typedef struct {
    command_type_t cmd_type;
    uint8_t data[SPP_DATA_MAX_LEN];
    size_t  len;
} spp_data_packet_t;

QueueHandle_t spp_data_queue;

void command_process_task(void *param);
#endif /* MAIN_COMMAND_PROCESS_H_ */