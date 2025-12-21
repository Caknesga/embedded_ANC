#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/timer.h"

#define LED_GPIO GPIO_NUM_9
#define LED_GPIO_YELLOW GPIO_NUM_10

#define TIMER_GROUP    TIMER_GROUP_0
#define TIMER_IDX      TIMER_0
#define TIMER_DIVIDER  80        // 80 MHz / 80 = 1 MHz (1 us tick)
#define TIMER_INTERVAL_US 1000000 // 500 ms

static bool led_state = false;

static void IRAM_ATTR timer_isr(void *arg)
{
    timer_group_clr_intr_status_in_isr(TIMER_GROUP, TIMER_IDX);
    timer_group_enable_alarm_in_isr(TIMER_GROUP, TIMER_IDX);

    led_state = !led_state;
    gpio_set_level(LED_GPIO, led_state);
    gpio_set_level(LED_GPIO_YELLOW, !led_state);
}

void app_main(void)
{
    // LED GPIO setup
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 0);

    gpio_reset_pin(LED_GPIO_YELLOW);
    gpio_set_direction(LED_GPIO_YELLOW, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO_YELLOW, 0);

    // Timer configuration
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = true,
    };

    timer_init(TIMER_GROUP, TIMER_IDX, &config);
    timer_set_counter_value(TIMER_GROUP, TIMER_IDX, 0);
    timer_set_alarm_value(TIMER_GROUP, TIMER_IDX, TIMER_INTERVAL_US);
    timer_enable_intr(TIMER_GROUP, TIMER_IDX);
    timer_isr_register(
        TIMER_GROUP,
        TIMER_IDX,
        timer_isr,
        NULL,
        ESP_INTR_FLAG_IRAM,
        NULL
    );

    timer_start(TIMER_GROUP, TIMER_IDX);

    // Main task does nothing
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
