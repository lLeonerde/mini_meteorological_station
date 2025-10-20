#include "tagoIO_communication.h"
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "esp_http_client.h"
#include "esp_tls.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"


#define DEVICE_TOKEN "bcc7b1c5-18c3-4ecf-9aab-e5754366bc41"
static const char *TAG = "HTTP_CLIENT";


esp_err_t _http_event_handler_tago(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "Resposta do Servidor:");
            printf("%.*s\n", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        default:
            break;
    }
    return ESP_OK;
}

esp_err_t send_tagoIO_data(float temperature, float pressure)
{
    char post_data[256];
     sprintf(post_data,
        "["
        "{"
            "\"variable\":\"temperature\","
            "\"value\":%.2f,"
            "\"unit\":\"C\""
        "},"
        "{"
            "\"variable\":\"pressure\","
            "\"value\":%.2f,"
            "\"unit\":\"hPa\""
        "}"
        "]",
        temperature, pressure
    );

    esp_http_client_config_t config = {
        .url = "https://api.us-e1.tago.io/data",
        .event_handler = _http_event_handler_tago,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Device-Token", DEVICE_TOKEN);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTPS Status = %d", esp_http_client_get_status_code(client));
        if (esp_http_client_get_status_code(client) >= 200 && esp_http_client_get_status_code(client) < 300) {
            ESP_LOGI(TAG, "Dados enviados com SUCESSO para o Tago.io!");
        } else {
            ESP_LOGE(TAG, "Falha ao enviar dados, Tago.io respondeu com código de erro.");
        }
    } else {
        ESP_LOGE(TAG, "Erro na requisição HTTPS: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    return err;
    
}