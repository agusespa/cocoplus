#include "distance.h"

#include <string.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "soc/gpio_num.h"

static const char *TAG = "DISTANCE_SENSORS";

#define TRIG_PIN_LEFT GPIO_NUM_22
#define ECHO_PIN_LEFT GPIO_NUM_23
#define TRIG_PIN_FRONT GPIO_NUM_18
#define ECHO_PIN_FRONT GPIO_NUM_19
#define TRIG_PIN_RIGHT GPIO_NUM_32
#define ECHO_PIN_RIGHT GPIO_NUM_33
#define SOUND_SPEED 0.034
#define TIMEOUT_US 50000

#define FILTER_SIZE 5
typedef struct {
    float buffer[FILTER_SIZE];
    int index;
} DistanceFilter;

float latest_distances[3] = {0.0, 0.0, 0.0};
SemaphoreHandle_t distance_mutex = NULL;

static DistanceFilter filters[3] = {{{0}, 0}};

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

void send_pulse(gpio_num_t trig_pin) {
    gpio_set_level(trig_pin, 1);
    esp_rom_delay_us(10);
    gpio_set_level(trig_pin, 0);
}

float measure_distance(gpio_num_t trig_pin, gpio_num_t echo_pin) {
    send_pulse(trig_pin);
    uint32_t pulse_duration = measure_pulse(echo_pin);
    return (pulse_duration * SOUND_SPEED) / 2.0;
}

static float get_median(float *buffer, int size) {
    float temp[FILTER_SIZE];
    memcpy(temp, buffer, sizeof(float) * size);
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (temp[j] > temp[j + 1]) {
                float t = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = t;
            }
        }
    }
    return temp[size / 2];
}

static float update_filter(DistanceFilter *f, float new_val) {
    f->buffer[f->index] = new_val;
    f->index = (f->index + 1) % FILTER_SIZE;
    return get_median(f->buffer, FILTER_SIZE);
}

float get_filtered_distance(int sensor_index, gpio_num_t trig_pin,
                            gpio_num_t echo_pin) {
    float raw = measure_distance(trig_pin, echo_pin);
    return update_filter(&filters[sensor_index], raw);
}

static void distance_update_task(void *param) {
    float temp[3];

    while (1) {
        temp[0] = get_filtered_distance(0, TRIG_PIN_LEFT, ECHO_PIN_LEFT);
        temp[1] = get_filtered_distance(1, TRIG_PIN_FRONT, ECHO_PIN_FRONT);
        temp[2] = get_filtered_distance(2, TRIG_PIN_RIGHT, ECHO_PIN_RIGHT);

        if (xSemaphoreTake(distance_mutex, portMAX_DELAY)) {
            memcpy(latest_distances, temp, sizeof(temp));
            xSemaphoreGive(distance_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

esp_err_t distance_sensors_init() {
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

    xTaskCreate(distance_update_task, "distance_update_task", 2048, NULL, 8, NULL);

    return ESP_OK;
}
