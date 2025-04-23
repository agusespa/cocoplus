#include "drive.h"

#include "../components/motors/driver.h"
#include "../components/sensors/distance.h"
#include "../main/cocoplus_main.h"
#include <string.h>

static const char* TAG = "DRIVE_TASK";

#define FORWARD_DISTANCE_THRESHOLD 30.0f

void drive_task(void* pvParameters) {
    float distances[3];

    while (1) {
        if (xSemaphoreTake(distance_mutex, portMAX_DELAY)) {
            memcpy(distances, latest_distances, sizeof(distances));
            xSemaphoreGive(distance_mutex);
        }

        if (!should_stop) {
            if (latest_distances[1] > FORWARD_DISTANCE_THRESHOLD) {
                set_motor_direction(MOTOR_LEFT, MOTOR_FORWARD);
                set_motor_direction(MOTOR_RIGHT, MOTOR_FORWARD);
            } else {
                set_motor_direction(MOTOR_LEFT, MOTOR_STOP);
                set_motor_direction(MOTOR_RIGHT, MOTOR_STOP);
            }
        } else {
            set_motor_direction(MOTOR_LEFT, MOTOR_STOP);
            set_motor_direction(MOTOR_RIGHT, MOTOR_STOP);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
