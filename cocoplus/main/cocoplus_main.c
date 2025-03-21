#include <stdio.h>

#include "components/mqtt_client/my_mqtt_client.h"
#include "components/wifi_utils/wifi_utils.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

static const char* TAG = "MQTT_MAIN";

#define SENSOR_PIN GPIO_NUM_4

void log_pub_error_message(esp_err_t ret) {
    char error_message[256];

    const char* error_name = esp_err_to_name(ret);
    if (error_name != NULL) {
        snprintf(error_message, sizeof(error_message), "Failed to configure sensor GPIO: %s", error_name);
    } else {
        snprintf(error_message, sizeof(error_message), "Failed to configure sensor GPIO: Unknown Error Code");
    }

    ESP_LOGE(TAG, "%s", error_message);
    mqtt_publish("cocoplus/health", error_message);
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

    gpio_config_t sensor_config = {.mode = GPIO_MODE_INPUT,
                                   .pull_up_en = GPIO_PULLUP_DISABLE,
                                   .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                   .intr_type = GPIO_INTR_DISABLE,
                                   .pin_bit_mask = (1ULL << SENSOR_PIN)};
    esp_err_t ret = gpio_config(&sensor_config);
    if (ret != ESP_OK) {
        log_pub_error_message(ret);
        return;
    }

    mqtt_publish("cocoplus/health", "GPIO configured successfully");

    while (1) {
        int buttonState = gpio_get_level(SENSOR_PIN);
        mqtt_publish("cocoplus/health", buttonState);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
