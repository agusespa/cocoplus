#ifndef MAIN_H
#define MAIN_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "soc/gpio_num.h"

extern bool should_stop;
extern SemaphoreHandle_t controller_mutex;

void send_pulse(gpio_num_t trig_pin);
uint32_t measure_pulse(gpio_num_t echo_pin);
void log_pub_error_message(esp_err_t ret);
void publish_data(float distance);
void obstacle_avoidance_task(void* pvParameters);
void mqtt_publish_task(void* pvParameters);

#endif  // MAIN_H
