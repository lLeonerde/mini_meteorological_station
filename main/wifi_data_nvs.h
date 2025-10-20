#ifndef WIFI_DATA_NVS_H
#define WIFI_DATA_NVS_H
#include "esp_err.h"
#include "wifi_structure.h"
#include "nvs_flash.h"
bool get_saved_config(nvs_handle_t my_handle, my_wifi_config_t *config);
esp_err_t save_wifi_config(nvs_handle_t my_handle, const my_wifi_config_t *config);
#endif