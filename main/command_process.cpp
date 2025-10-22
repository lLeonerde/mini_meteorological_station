#include "command_process.h"
#include "wifi_data_nvs.h"
#include "app_globals.h"
#include <string.h>
#include <ctype.h>
EventGroupHandle_t app_event_group;

// command will be:
// cmd=type,data
// example: cmd=wifi_cfg,My_SSID,My_Password
void command_process_task(void *param){
    spp_data_packet_t data_received;

    for(;;){
        if(xQueueReceive(spp_data_queue, &data_received, portMAX_DELAY) == pdTRUE){
            char buffer_local[SPP_DATA_MAX_LEN + 1];
            memcpy(buffer_local, data_received.data, data_received.len);
            buffer_local[data_received.len] = '\0';
            //process command and extract cmd_type
            if(strncmp(buffer_local, "cmd=wifi_cfg,", 13) == 0){
                data_received.cmd_type = CMD_SET_WIFI_CFG;
            } else {
                data_received.cmd_type = CMD_UNKNOWN;
                continue;
            }
            switch(data_received.cmd_type){
                case CMD_SET_WIFI_CFG:
                {
                    char *saveptr;
                    char *data_start = buffer_local + 13;
                    char *ssid = strtok_r(data_start, ",", &saveptr);
                    char *password = strtok_r(NULL, ",", &saveptr);
                    printf("DEBUG: Parser extraiu -> SSID: [%s], Senha: [%s]\n", ssid, password);
                    if (ssid != NULL && password != NULL) {

                        size_t len = strlen(password);
                        while (len > 0 && (password[len - 1] == '\n' || password[len - 1] == '\r' || isspace(password[len - 1]))) {
                            password[--len] = '\0'; 
                        }
                        my_wifi_config_t new_config;
                        strncpy(new_config.ssid, ssid, sizeof(new_config.ssid) - 1);
                        strncpy(new_config.password, password, sizeof(new_config.password) - 1);
                        new_config.ssid[sizeof(new_config.ssid) - 1] = '\0';
                        new_config.password[sizeof(new_config.password) - 1] = '\0';
                        esp_err_t err = save_wifi_config(my_nvs_handle, &new_config);
                        if(err == ESP_OK){
                            printf("comando processado\n");
                            xEventGroupSetBits(app_event_group, WIFI_CONFIG_UPDATED_BIT);
                        }
                    } else {
                        printf("Erro de sintaxe no comando wifi_cfg. Formato esperado: cmd=wifi_cfg,SSID,SENHA\r\n");
                    }
                    break;
                }
                case CMD_UNKNOWN:
                    break;
                default:
                    break;
            }
        }
    }
}
