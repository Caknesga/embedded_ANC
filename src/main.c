#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/uart.h"
#include <unistd.h>
// ================= CONFIG =================
#define TAG "ESP32_AUDIO_USB"

// GPIO pins for INMP441
#define I2S_BCLK   GPIO_NUM_6
#define I2S_WS     GPIO_NUM_7
#define I2S_SD     GPIO_NUM_21

// Audio
#define SAMPLE_RATE    16000
#define FRAME_SAMPLES  256

// I2S
#define I2S_PORT I2S_NUM_0
// =========================================

// I2S raw buffer
static int32_t i2s_rx_buffer[FRAME_SAMPLES];

// Audio frame to send
static int16_t audio_frame[FRAME_SAMPLES];

// ---------- I2S INIT ----------
static void i2s_init(void)
{
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .dma_buf_count = 4,
        .dma_buf_len = FRAME_SAMPLES,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };

    ESP_ERROR_CHECK(i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_PORT, &pin_config));
    ESP_ERROR_CHECK(i2s_zero_dma_buffer(I2S_PORT));
}

// ---------- MAIN ----------
void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_NONE);

    i2s_init();

    size_t bytes_read;

    while (1) {
        // Read I2S frame
        esp_err_t err = i2s_read(
            I2S_PORT,
            i2s_rx_buffer,
            sizeof(i2s_rx_buffer),
            &bytes_read,
            portMAX_DELAY
        );

        if (err != ESP_OK || bytes_read == 0) {
            continue;
        }

        int samples = bytes_read / sizeof(int32_t);

        // Convert to int16
        for (int i = 0; i < samples; i++) {
            audio_frame[i] = (int16_t)(i2s_rx_buffer[i] >> 16);
        }

        // 🔥 Send raw audio via USB CDC (stdout)
        write(STDOUT_FILENO, audio_frame, samples * sizeof(int16_t));
    }
}