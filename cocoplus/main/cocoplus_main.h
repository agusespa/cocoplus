#ifndef MAIN_H
#define MAIN_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

extern bool should_stop;
extern SemaphoreHandle_t controller_mutex;

void send_pulse();
uint32_t measure_pulse();
void log_pub_error_message(esp_err_t ret);
void publish_data(float distance);
void obstacle_avoidance_task(void* pvParameters);
void mqtt_publish_task(void* pvParameters);

#endif  // MAIN_H
