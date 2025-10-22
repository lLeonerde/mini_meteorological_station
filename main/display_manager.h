#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H
#include <LovyanGFX.hpp>
#include "app_globals.h"
#include "wifi_structure.h"
typedef enum {
    DISPLAY_STATE_BOOTING,
    DISPLAY_STATE_SHOW_DATA,
    DISPLAY_STATE_BT_WAITING,
    DISPLAY_STATE_BT_CONNECTED,
    DISPLAY_STATE_WIFI_CONNECTING,
    DISPLAY_STATE_CLEAR,
} display_state_t;

typedef struct {
    display_state_t state;
} display_message_t;

extern QueueHandle_t display_queue;

void display_task(void *param);

#endif // DISPLAY_MANAGER_H