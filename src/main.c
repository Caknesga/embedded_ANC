#include <stdio.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#define LED_GPIO      GPIO_NUM_9
#define ADC_UNIT      ADC_UNIT_1
#define ADC_CH        ADC_CHANNEL_2   // GPIO2 (A0)
#define CLAP_THRESH   200              // tune this
#define SILENCE_THRESH 80              // must be lower

void app_main(void)
{
    // ---- LED ----
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 0);

    // ---- ADC ----
    adc_oneshot_unit_handle_t adc_handle;
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT
    };
    adc_oneshot_new_unit(&unit_cfg, &adc_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_11
    };
    adc_oneshot_config_channel(adc_handle, ADC_CH, &chan_cfg);

    // ---- Baseline ----
    int baseline = 0, raw;
    for (int i = 0; i < 200; i++) {
        adc_oneshot_read(adc_handle, ADC_CH, &raw);
        baseline += raw;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    baseline /= 200;

    // ---- State ----
    bool led_on = false;
    bool clap_ready = true;

    while (1) {
        adc_oneshot_read(adc_handle, ADC_CH, &raw);

        int diff = raw - baseline;
        if (diff < 0) diff = -diff;

        // ---- Clap detection (EDGE) ----
        if (clap_ready && diff > CLAP_THRESH) {
            led_on = !led_on;
            gpio_set_level(LED_GPIO, led_on);
            clap_ready = false;   // block until silence
        }

        // ---- Re-arm after silence ----
        if (!clap_ready && diff < SILENCE_THRESH) {
            clap_ready = true;
        }

        vTaskDelay(pdMS_TO_TICKS(2));  // ~500 Hz sampling
    }
}