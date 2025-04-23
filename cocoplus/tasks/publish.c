#include "publish.h"

#include "../components/mqtt_client/my_mqtt_client.h"
#include "../components/sensors/distance.h"
#include "../main/cocoplus_main.h"

static const char* TAG = "MQTT_TASK";
static const char* SENSOR_DATA_TOPIC = "cocoplus/data";

void publish_task(void* pvParameters) {
    float distances[3];

    while (1) {
        if (xSemaphoreTake(distance_mutex, portMAX_DELAY)) {
            memcpy(distances, latest_distances, sizeof(distances));
            xSemaphoreGive(distance_mutex);
        }

        char message[50];
        snprintf(message, sizeof(message), "l:%.2f;f:%.2f;r:%.2f", distances[0],
                 distances[1], distances[2]);
        mqtt_publish(SENSOR_DATA_TOPIC, message);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
