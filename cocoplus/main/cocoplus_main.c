#include "cocoplus_main.h"

#include <stdio.h>

#include "components/mqtt_client/my_mqtt_client.h"
#include "components/wifi_utils/wifi_utils.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/task.h"
#include "nvs_flash.h"

static const char* TAG = "COCO_MAIN";

#define TRIG_PIN_LEFT GPIO_NUM_22
#define ECHO_PIN_LEFT GPIO_NUM_23
#define TRIG_PIN_FRONT GPIO_NUM_18
#define ECHO_PIN_FRONT GPIO_NUM_19
#define TRIG_PIN_RIGHT GPIO_NUM_32
#define ECHO_PIN_RIGHT GPIO_NUM_33
#define SOUND_SPEED 0.034
#define TIMEOUT_US 50000

static const char* SENSOR_DATA_TOPIC = "cocoplus/data";
static const char* HEALTH_TOPIC = "cocoplus/health";

static TaskHandle_t obstacle_avoidance_task_handle = NULL;
static TaskHandle_t mqtt_publish_task_handle = NULL;

static float latest_distances[3] = {0.0, 0.0, 0.0};
static SemaphoreHandle_t distance_mutex;

bool should_stop = true;
SemaphoreHandle_t controller_mutex;

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
            ESP_LOGE("measure_pulse", "Timeout waiting for pulse start");
            return 0;
        }
    }

    start_time = esp_timer_get_time();
    timeout_time = start_time + TIMEOUT_US;

    while (gpio_get_level(echo_pin) == 1) {
        if (esp_timer_get_time() > timeout_time) {
            ESP_LOGE("measure_pulse", "Timeout waiting for pulse end");
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

void obstacle_avoidance_task(void* pvParameters) {
    while (1) {
        float distances[3];
        distances[0] = measure_distance(TRIG_PIN_LEFT, ECHO_PIN_LEFT);
        distances[1] = measure_distance(TRIG_PIN_FRONT, ECHO_PIN_FRONT);
        distances[2] = measure_distance(TRIG_PIN_RIGHT, ECHO_PIN_RIGHT);

        if (xSemaphoreTake(distance_mutex, portMAX_DELAY)) {
            for (int i = 0; i < 3; i++) {
                latest_distances[i] = distances[i];
            }
            xSemaphoreGive(distance_mutex);
        }

        if (should_stop) {
            // TODO: Implement start/stop logic
            /* stop_movement(); */
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void mqtt_publish_task(void* pvParameters) {
    while (1) {
        float left_distance = 0.0, front_distance = 0.0, right_distance = 0.0;

        if (xSemaphoreTake(distance_mutex, portMAX_DELAY)) {
            left_distance = latest_distances[0];
            front_distance = latest_distances[1];
            right_distance = latest_distances[2];
            xSemaphoreGive(distance_mutex);
        }

        char message[50];
        snprintf(message, sizeof(message), "l:%.2f;f:%.2f;r:%.2f",
                 left_distance, front_distance, right_distance);
        mqtt_publish(SENSOR_DATA_TOPIC, message);

        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

void app_main() {
    esp_err_t nvs_err = nvs_flash_init();
    if (nvs_err == ESP_ERR_NVS_NO_FREE_PAGES ||
        nvs_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_err);

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
        ESP_LOGE(TAG, "Failed to create mutex");
        abort();
    }

    controller_mutex = xSemaphoreCreateMutex();
    if (controller_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        abort();
    }

    ESP_LOGI(TAG, "Connecting to Wi-Fi...");
    wifi_status_t status = wifi_init_sta();
    if (status == WIFI_STATUS_FAIL) {
        ESP_LOGE("APP", "Wi-Fi connection failed. Stopping execution.");
        abort();
    }

    esp_err_t mqtt_err = mqtt_app_start();
    if (mqtt_err != ESP_OK) {
        ESP_LOGE(TAG, "MQTT initialization failed. Stopping execution.");
        abort();
    }

    mqtt_publish(HEALTH_TOPIC, "Startup completed successfully");

    xTaskCreate(obstacle_avoidance_task, "ObstacleAvoidanceTask", 4096, NULL,
                10, &obstacle_avoidance_task_handle);
    xTaskCreate(mqtt_publish_task, "MQTTPublishTask", 4096, NULL, 3,
                &mqtt_publish_task_handle);
}

