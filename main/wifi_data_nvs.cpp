#include "wifi_data_nvs.h"
#include "wifi_structure.h"
#include "nvs_flash.h"

bool get_saved_config(nvs_handle_t my_handle, my_wifi_config_t *config){
    size_t required_size;

    nvs_get_str(my_handle, "ssid", NULL, &required_size);
    if(required_size <= sizeof(config->ssid)){
        nvs_get_str(my_handle, "ssid", config->ssid, &required_size);
    } else {
        config->ssid[0] = '\0';
    }

    nvs_get_str(my_handle, "password", NULL, &required_size);
    if(required_size <= sizeof(config->password)){
        nvs_get_str(my_handle, "password", config->password, &required_size);
    } else {
        config->password[0] = '\0'; 
    }

    if(config->ssid[0] != '\0' && config->password[0] != '\0'){
        return true;
    } else {
        return false;
    }
}

esp_err_t save_wifi_config(nvs_handle_t my_handle, const my_wifi_config_t *config){
    esp_err_t err;

    err = nvs_set_str(my_handle, "ssid", config->ssid);
    if(err != ESP_OK){
        return err;
    }

    err = nvs_set_str(my_handle, "password", config->password);
    if(err != ESP_OK){
        return err;
    }

    err = nvs_commit(my_handle);
    return err;
}