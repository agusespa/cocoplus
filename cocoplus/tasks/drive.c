#include "../components/sensors/sensors.h"
#include "../main/cocoplus_main.h"
#include "drive.h"

static const char* TAG = "DRIVE_TASK";

void drive_task(void* pvParameters) {
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
