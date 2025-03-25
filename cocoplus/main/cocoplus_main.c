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

#define TRIG_PIN GPIO_NUM_14
#define ECHO_PIN GPIO_NUM_15
#define SOUND_SPEED 0.034
#define TIMEOUT_US 50000

static const char* SENSOR_DATA_TOPIC = "cocoplus/data";
static const char* HEALTH_TOPIC = "cocoplus/health";

static TaskHandle_t obstacle_avoidance_task_handle = NULL;
static TaskHandle_t mqtt_publish_task_handle = NULL;

static float latest_distance = 0.0;
static SemaphoreHandle_t distance_mutex;

bool should_stop = true;
SemaphoreHandle_t controller_mutex;

void send_pulse() {
    gpio_set_level(TRIG_PIN, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIG_PIN, 0);
}

uint32_t measure_pulse() {
    uint32_t start_time = esp_timer_get_time();
    uint32_t timeout_time = start_time + TIMEOUT_US;

    while (gpio_get_level(ECHO_PIN) == 0) {
        if (esp_timer_get_time() > timeout_time) {
            ESP_LOGE("measure_pulse", "Timeout waiting for pulse start");
            return 0;
        }
    }

    start_time = esp_timer_get_time();
    timeout_time = start_time + TIMEOUT_US;

    while (gpio_get_level(ECHO_PIN) == 1) {
        if (esp_timer_get_time() > timeout_time) {
            ESP_LOGE("measure_pulse", "Timeout waiting for pulse end");
            return 0;
        }
    }

    uint32_t end_time = esp_timer_get_time();
    return end_time - start_time;
}

void log_pub_error_message(esp_err_t ret) {
    char error_message[256];

    const char* error_name = esp_err_to_name(ret);
    if (error_name != NULL) {
        snprintf(error_message, sizeof(error_message),
                 "Failed to configure sensor GPIO: %s", error_name);
    } else {
        snprintf(error_message, sizeof(error_message),
                 "Failed to configure sensor GPIO: Unknown Error Code");
    }

    ESP_LOGE(TAG, "%s", error_message);
    mqtt_publish(HEALTH_TOPIC, error_message);
}

void obstacle_avoidance_task(void* pvParameters) {
    while (1) {
        send_pulse();
        uint32_t pulse_duration = measure_pulse();
        float distance = (pulse_duration * SOUND_SPEED) / 2.0;

        if (xSemaphoreTake(distance_mutex, portMAX_DELAY)) {
            latest_distance = distance;
            xSemaphoreGive(distance_mutex);
        }

        // TODO: Implement logic to control the car's movement
        if (should_stop) {
            // TODO: implement start/stop
            /* stop_movement(); */
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void mqtt_publish_task(void* pvParameters) {
    float front_distance = 0.0;
    float left_distance = 0.0;
    float right_distance = 0.0;

    while (1) {
        if (xSemaphoreTake(distance_mutex, portMAX_DELAY)) {
            front_distance = latest_distance;
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

    ESP_LOGI(TAG, "Connecting to wi-fi ESP_WIFI_MODE_STA...");
    wifi_status_t status = wifi_init_sta();
    if (status == WIFI_STATUS_FAIL) {
        ESP_LOGE(TAG, "Wi-Fi connection failed. Stopping execution.");
        return;
    }

    esp_err_t mqtt_err = mqtt_app_start();
    if (mqtt_err != ESP_OK) {
        ESP_LOGE(TAG, "MQTT initialization failed. Stopping execution.");
        return;
    }

    gpio_config_t trig_config = {.mode = GPIO_MODE_OUTPUT,
                                 .pull_up_en = GPIO_PULLUP_DISABLE,
                                 .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                 .intr_type = GPIO_INTR_DISABLE,
                                 .pin_bit_mask = (1ULL << TRIG_PIN)};
    esp_err_t ret = gpio_config(&trig_config);
    if (ret != ESP_OK) {
        log_pub_error_message(ret);
        return;
    }

    gpio_config_t echo_config = {.mode = GPIO_MODE_INPUT,
                                 .pull_up_en = GPIO_PULLUP_DISABLE,
                                 .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                 .intr_type = GPIO_INTR_DISABLE,
                                 .pin_bit_mask = (1ULL << ECHO_PIN)};
    ret = gpio_config(&echo_config);
    if (ret != ESP_OK) {
        log_pub_error_message(ret);
        return;
    }

    distance_mutex = xSemaphoreCreateMutex();
    if (distance_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return;
    }

    controller_mutex = xSemaphoreCreateMutex();
    if (controller_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return;
    }

    mqtt_publish(HEALTH_TOPIC, "startup completed successfully");

    xTaskCreate(obstacle_avoidance_task, "ObstacleAvoidanceTask", 4096, NULL,
                10, &obstacle_avoidance_task_handle);

    xTaskCreate(mqtt_publish_task, "MQTTPublishTask", 4096, NULL, 3,
                &mqtt_publish_task_handle);
}
