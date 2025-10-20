#ifndef TAGOIO_COMMUNICATION_H
#define TAGOIO_COMMUNICATION_H
#include "esp_err.h"
extern "C"
{
    esp_err_t send_tagoIO_data(float temperature, float pressure);
}
#endif