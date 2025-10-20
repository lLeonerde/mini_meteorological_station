#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H
#include "esp_err.h"
#include "wifi_structure.h"
extern "C"
{
esp_err_t wifi_init_sta(my_wifi_config_t my_wifi_config);
}
#endif