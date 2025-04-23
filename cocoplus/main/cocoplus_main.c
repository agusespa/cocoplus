#include "cocoplus_main.h"

#include <stdio.h>

#include "../components/motors/driver.h"
#include "../components/mqtt_client/my_mqtt_client.h"
#include "../components/sensors/distance.h"
#include "../components/wifi_utils/wifi_utils.h"
#include "../tasks/drive.h"
#include "../tasks/publish.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char* TAG = "COCO_MAIN";
static const char* HEALTH_TOPIC = "cocoplus/health";

TaskHandle_t drive_task_handle = NULL;
TaskHandle_t publish_task_handle = NULL;
SemaphoreHandle_t controller_mutex = NULL;
bool should_stop = true;

void app_main() {
    esp_err_t nvs_err = nvs_flash_init();
    if (nvs_err == ESP_ERR_NVS_NO_FREE_PAGES ||
        nvs_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_err);

    esp_err_t sensor_init_result = distance_sensors_init();
    if (sensor_init_result != ESP_OK) {
        ESP_LOGE(TAG, "Sensor initialization failed. Stopping main execution.");
        abort();
    }

    esp_err_t motor_init_result = motor_init();
    if (motor_init_result != ESP_OK) {
        ESP_LOGE(TAG, "Motor initialization failed. Stopping main execution.");
        return;
    }

    controller_mutex = xSemaphoreCreateMutex();
    if (controller_mutex == NULL) {
        ESP_LOGE(TAG,
                 "Failed to create controller mutex. Stopping main execution.");
        abort();
    }

    ESP_LOGI(TAG, "Connecting to Wi-Fi...");
    wifi_status_t status = wifi_init_sta();
    if (status == WIFI_STATUS_FAIL) {
        ESP_LOGE(TAG, "Wi-Fi connection failed. Stopping main execution.");
        abort();
    }

    esp_err_t mqtt_err = mqtt_app_start();
    if (mqtt_err != ESP_OK) {
        ESP_LOGE(TAG, "MQTT initialization failed. Stopping main execution.");
        abort();
    }

    mqtt_publish(HEALTH_TOPIC, "Startup completed successfully");

    xTaskCreate(drive_task, "DriveTask", 4096, NULL, 10, &drive_task_handle);
    xTaskCreate(publish_task, "MQTTPublishTask", 4096, NULL, 3,
                &publish_task_handle);
}
