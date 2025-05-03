#include "drive.h"

#include <string.h>

#include "../components/motors/driver.h"
#include "../components/sensors/distance.h"
#include "../main/cocoplus_main.h"

static const char* TAG = "DRIVE_TASK";

#define FORWARD_DISTANCE_THRESHOLD 40.0f
#define MIN_DISTANCE 20.0f

static int compute_proportional_speed(float distance_cm) {
    if (distance_cm <= MIN_DISTANCE) {
        return 0;
    } else if (distance_cm >= FORWARD_DISTANCE_THRESHOLD) {
        return BASE_SPEED;
    } else {
        float ratio = (distance_cm - MIN_DISTANCE) /
                      (FORWARD_DISTANCE_THRESHOLD - MIN_DISTANCE);
        return (int)(BASE_SPEED * ratio);
    }
}

void drive_task(void* pvParameters) {
    float distances[3];

    while (1) {
        if (xSemaphoreTake(distance_mutex, portMAX_DELAY)) {
            memcpy(distances, latest_distances, sizeof(distances));
            xSemaphoreGive(distance_mutex);
        }

        if (!should_stop) {
            float front_distance = distances[1];
            int speed = compute_proportional_speed(front_distance);
            drive_robot(speed, 0);
        } else {
            drive_robot(0, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
