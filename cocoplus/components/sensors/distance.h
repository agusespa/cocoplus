#ifndef DISTANCE_SENSORS_H
#define DISTANCE_SENSORS_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

extern float latest_distances[3];
extern SemaphoreHandle_t distance_mutex;

esp_err_t distance_sensors_init();

#endif  // DISTANCE_SENSORS_H
