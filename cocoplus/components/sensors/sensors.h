#ifndef SENSORS_H
#define SENSORS_H

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define TRIG_PIN_LEFT GPIO_NUM_22
#define ECHO_PIN_LEFT GPIO_NUM_23
#define TRIG_PIN_FRONT GPIO_NUM_18
#define ECHO_PIN_FRONT GPIO_NUM_19
#define TRIG_PIN_RIGHT GPIO_NUM_32
#define ECHO_PIN_RIGHT GPIO_NUM_33
#define SOUND_SPEED 0.034
#define TIMEOUT_US 50000

extern float latest_distances[3];
extern SemaphoreHandle_t distance_mutex;

esp_err_t sensors_init();
void send_pulse(gpio_num_t trig_pin);
uint32_t measure_pulse(gpio_num_t echo_pin);
float measure_distance(gpio_num_t trig_pin, gpio_num_t echo_pin);

#endif  // SENSORS_H
