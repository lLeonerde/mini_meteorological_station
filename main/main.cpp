#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>

// --- INCLUDES DAS BIBLIOTECAS ---
#include <LovyanGFX.hpp> // Para o Display
#include <bmp280.h>      // Para o Sensor

// --- 1. CONFIGURAÇÃO DA LOVYANGFX (a mesma de antes) ---
#define LGFX_USE_V1

class LGFX_OLED_I2C_128x64 : public lgfx::LGFX_Device
{
    lgfx::Panel_SSD1306 _panel_instance;
    lgfx::Bus_I2C       _bus_instance;

public:
    LGFX_OLED_I2C_128x64(void)
    {
        {
            auto cfg = _bus_instance.config();
            cfg.i2c_port    = 0;
            cfg.i2c_addr    = 0x3C; // Endereço I2C do Display
            cfg.pin_sda     = 5;
            cfg.pin_scl     = 4;
            cfg.freq_write  = 400000;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();
            cfg.pin_rst     = -1;
            cfg.panel_width = 128;
            cfg.panel_height = 64;
            _panel_instance.config(cfg);
        }
        setPanel(&_panel_instance);
    }
};

// --- 2. INSTÂNCIAS GLOBAIS DOS NOSSOS DISPOSITIVOS ---
LGFX_OLED_I2C_128x64 lcd;       // Nosso display
LGFX_Sprite canvas(&lcd);     // Nosso buffer de tela (Sprite)
bmp280_t sensor_dev;          // Nossa estrutura do sensor

extern "C"
{
    void app_main(void)
    {
        // --- 3. INICIALIZAÇÃO DO BARRAMENTO I2C ---
        // É importante inicializar o driver I2C do ESP-IDF antes de qualquer dispositivo
        ESP_ERROR_CHECK(i2cdev_init());

        // --- 4. INICIALIZAÇÃO DO SENSOR BMP280 ---
        bmp280_params_t params;
        bmp280_init_default_params(&params);
        memset(&sensor_dev, 0, sizeof(bmp280_t));
        // Use os mesmos pinos SDA e SCL que definimos para o display!
        ESP_ERROR_CHECK(bmp280_init_desc(&sensor_dev, BMP280_I2C_ADDRESS_0, I2C_NUM_0, (gpio_num_t)5, (gpio_num_t)4));
        ESP_ERROR_CHECK(bmp280_init(&sensor_dev, &params));

        bool is_bme280 = (sensor_dev.id == BME280_CHIP_ID);
        printf("Sensor encontrado: %s\n", is_bme280 ? "BME280" : "BMP280");


        // --- 5. INICIALIZAÇÃO DO DISPLAY ---
        lcd.init();
        canvas.setColorDepth(1);
        canvas.createSprite(128, 64);


        // --- 6. LOOP PRINCIPAL ---
        float pressure, temperature, humidity;

        while (1)
        {
            // A. LER DADOS DO SENSOR
            if (bmp280_read_float(&sensor_dev, &temperature, &pressure, &humidity) != ESP_OK)
            {
                printf("Leitura do sensor falhou\n");
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }

            // B. DESENHAR TUDO NO CANVAS (na memória)
            canvas.fillScreen(TFT_BLACK); // Limpa o buffer
            canvas.setTextColor(TFT_WHITE);

            // Desenha a Temperatura
            canvas.setTextSize(2);
            canvas.setCursor(5, 5);
            canvas.printf("%.1f C", temperature);
            canvas.drawCircle(88, 7, 2, TFT_WHITE); // Símbolo de grau °

            // Desenha a Pressão
            canvas.setTextSize(1);
            canvas.setCursor(5, 30);
            canvas.printf("Press: %.0f hPa", pressure / 100.0f); // Converte de Pa para hPa


            canvas.pushSprite(0, 0);
 
            vTaskDelay(pdMS_TO_TICKS(2000)); // Atualiza a cada 2 segundos
        }
    }
}