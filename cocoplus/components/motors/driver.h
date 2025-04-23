#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include "driver/gpio.h"
#include "esp_err.h"

#define MOTOR0_PIN1 GPIO_NUM_14  // Left Motor direction pin 1
#define MOTOR0_PIN2 GPIO_NUM_12  // Left Motor direction pin 2
#define MOTOR1_PIN1 GPIO_NUM_5  // Right Motor direction pin 1
#define MOTOR1_PIN2 GPIO_NUM_17  // Right Motor direction pin 2

typedef enum { MOTOR_LEFT = 0, MOTOR_RIGHT = 1 } motor_id_t;

typedef enum {
    MOTOR_FORWARD = 1,
    MOTOR_BACKWARD = -1,
    MOTOR_STOP = 0
} motor_direction_t;

esp_err_t motor_init();
void set_motor_direction(motor_id_t motor_id, motor_direction_t direction);

#endif  // MOTOR_DRIVER_H
