#include "../components/mqtt_client/my_mqtt_client.h"
#include "../components/sensors/sensors.h"
#include "../main/cocoplus_main.h"
#include "publish.h"

static const char* TAG = "MQTT_TASK";
static const char* SENSOR_DATA_TOPIC = "cocoplus/data";

void publish_task(void* pvParameters) {
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
