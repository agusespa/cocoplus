#include "sensors.h"

#include "esp_log.h"

static const char* TAG = "SENSORS";

float latest_distances[3] = {0.0, 0.0, 0.0};
SemaphoreHandle_t distance_mutex = NULL;

void send_pulse(gpio_num_t trig_pin) {
    gpio_set_level(trig_pin, 1);
    esp_rom_delay_us(10);
    gpio_set_level(trig_pin, 0);
}

uint32_t measure_pulse(gpio_num_t echo_pin) {
    uint32_t start_time = esp_timer_get_time();
    uint32_t timeout_time = start_time + TIMEOUT_US;

    while (gpio_get_level(echo_pin) == 0) {
        if (esp_timer_get_time() > timeout_time) {
            ESP_LOGE(TAG, "Timeout waiting for pulse start on pin %d",
                     echo_pin);
            return 0;
        }
    }

    start_time = esp_timer_get_time();
    timeout_time = start_time + TIMEOUT_US;

    while (gpio_get_level(echo_pin) == 1) {
        if (esp_timer_get_time() > timeout_time) {
            ESP_LOGE(TAG, "Timeout waiting for pulse end on pin %d", echo_pin);
            return 0;
        }
    }

    return esp_timer_get_time() - start_time;
}

float measure_distance(gpio_num_t trig_pin, gpio_num_t echo_pin) {
    send_pulse(trig_pin);
    uint32_t pulse_duration = measure_pulse(echo_pin);
    return (pulse_duration * SOUND_SPEED) / 2.0;
}

esp_err_t sensors_init() {
    gpio_config_t trig_config = {.mode = GPIO_MODE_OUTPUT,
                                 .pull_up_en = GPIO_PULLUP_DISABLE,
                                 .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                 .intr_type = GPIO_INTR_DISABLE,
                                 .pin_bit_mask = (1ULL << TRIG_PIN_LEFT) |
                                                 (1ULL << TRIG_PIN_FRONT) |
                                                 (1ULL << TRIG_PIN_RIGHT)};
    ESP_ERROR_CHECK(gpio_config(&trig_config));

    gpio_config_t echo_config = {.mode = GPIO_MODE_INPUT,
                                 .pull_up_en = GPIO_PULLUP_DISABLE,
                                 .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                 .intr_type = GPIO_INTR_DISABLE,
                                 .pin_bit_mask = (1ULL << ECHO_PIN_LEFT) |
                                                 (1ULL << ECHO_PIN_FRONT) |
                                                 (1ULL << ECHO_PIN_RIGHT)};
    ESP_ERROR_CHECK(gpio_config(&echo_config));

    distance_mutex = xSemaphoreCreateMutex();
    if (distance_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create distance mutex");
        return ESP_FAIL;
    }
    return ESP_OK;
}
