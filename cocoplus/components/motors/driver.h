#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include "driver/gpio.h"
#include "esp_err.h"

#define BASE_SPEED 768

esp_err_t motor_init();
void drive_robot(int linear_speed, int steering_diff);

#endif  // MOTOR_DRIVER_H
