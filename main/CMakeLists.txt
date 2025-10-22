idf_component_register(SRCS "cloud_task.cpp" "button_task.cpp" "sensor_task.cpp" "command_process.cpp" "wifi_data_nvs.cpp" "tagoIO_communication.cpp" "wifi_setup.cpp" "bluetooth.cpp" "main.cpp" "display_manager.cpp" "cloud_task.cpp"
                      INCLUDE_DIRS "."
                      REQUIRES LovyanGFX bmp280 bt esp_http_client esp-tls nvs_flash)
