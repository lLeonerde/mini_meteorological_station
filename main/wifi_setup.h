#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H
#include "esp_err.h"
#include "wifi_structure.h"
extern "C"
{
void wifi_init(void);
void wifi_connect(my_wifi_config_t my_wifi_config);
}
#endif