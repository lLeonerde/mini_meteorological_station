idf_component_register(SRCS "command_process.c" "wifi_data_nvs.cpp" "tagoIO_communication.cpp" "wifi_setup.cpp" "bluetooth.cpp" "main.cpp"
                      INCLUDE_DIRS "."
                      REQUIRES LovyanGFX bmp280 bt esp_http_client esp-tls nvs_flash)
