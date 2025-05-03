#include "driver.h"

#include "driver/ledc.h"
#include "esp_log.h"

static const char* TAG = "MOTOR_DRIVER";

#define MOTOR0_PIN1 GPIO_NUM_14  // Left Motor direction pin 1
#define MOTOR0_PIN2 GPIO_NUM_12  // Left Motor direction pin 2
#define MOTOR1_PIN1 GPIO_NUM_5   // Right Motor direction pin 1
#define MOTOR1_PIN2 GPIO_NUM_17  // Right Motor direction pin 2

#define LEDC_CHANNEL_M0_PIN1 LEDC_CHANNEL_0
#define LEDC_CHANNEL_M0_PIN2 LEDC_CHANNEL_1
#define LEDC_CHANNEL_M1_PIN1 LEDC_CHANNEL_2
#define LEDC_CHANNEL_M1_PIN2 LEDC_CHANNEL_3

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES LEDC_TIMER_10_BIT
#define LEDC_FREQUENCY 1000

#define MAX_DUTY ((1 << LEDC_DUTY_RES) - 1)  // 1023
#define MIN_DUTY 386

typedef enum { MOTOR_LEFT, MOTOR_RIGHT } motor_id_t;
typedef enum { FORWARD = 1, BACKWARD = -1, STOP = 0 } motor_direction_t;

static int normalize_duty(int speed) {
    if (speed == 0) return 0;

    if (speed > MAX_DUTY) speed = MAX_DUTY;
    if (speed < -MAX_DUTY) speed = -MAX_DUTY;

    if (speed > 0 && speed < MIN_DUTY) return MIN_DUTY;
    if (speed < 0 && speed > -MIN_DUTY) return -MIN_DUTY;

    return speed;
}

esp_err_t motor_init() {
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    ledc_channel_config_t channels[] = {
        {.channel = LEDC_CHANNEL_M0_PIN1,
         .gpio_num = MOTOR0_PIN1,
         .duty = 0,
         .timer_sel = LEDC_TIMER},
        {.channel = LEDC_CHANNEL_M0_PIN2,
         .gpio_num = MOTOR0_PIN2,
         .duty = 0,
         .timer_sel = LEDC_TIMER},
        {.channel = LEDC_CHANNEL_M1_PIN1,
         .gpio_num = MOTOR1_PIN1,
         .duty = 0,
         .timer_sel = LEDC_TIMER},
        {.channel = LEDC_CHANNEL_M1_PIN2,
         .gpio_num = MOTOR1_PIN2,
         .duty = 0,
         .timer_sel = LEDC_TIMER},
    };

    for (int i = 0; i < 4; i++) {
        channels[i].speed_mode = LEDC_MODE;
        channels[i].hpoint = 0;
        ESP_ERROR_CHECK(ledc_channel_config(&channels[i]));
    }

    return ESP_OK;
}

esp_err_t drive_motor(motor_id_t motor, motor_direction_t direction,
                      int speed) {
    speed = normalize_duty(speed);

    int pin1 =
        (motor == MOTOR_LEFT) ? LEDC_CHANNEL_M0_PIN1 : LEDC_CHANNEL_M1_PIN1;
    int pin2 =
        (motor == MOTOR_LEFT) ? LEDC_CHANNEL_M0_PIN2 : LEDC_CHANNEL_M1_PIN2;

    esp_err_t ret = ESP_OK;

    if (direction == STOP) {
        ret |= ledc_set_duty(LEDC_MODE, pin1, 0);
        ret |= ledc_update_duty(LEDC_MODE, pin1);
        ret |= ledc_set_duty(LEDC_MODE, pin2, 0);
        ret |= ledc_update_duty(LEDC_MODE, pin2);
    } else if (direction == FORWARD) {
        ret |= ledc_set_duty(LEDC_MODE, pin1, speed);
        ret |= ledc_update_duty(LEDC_MODE, pin1);
        ret |= ledc_set_duty(LEDC_MODE, pin2, 0);
        ret |= ledc_update_duty(LEDC_MODE, pin2);
    } else if (direction == BACKWARD) {
        ret |= ledc_set_duty(LEDC_MODE, pin1, 0);
        ret |= ledc_update_duty(LEDC_MODE, pin1);
        ret |= ledc_set_duty(LEDC_MODE, pin2, speed);
        ret |= ledc_update_duty(LEDC_MODE, pin2);
    }

    return ret;
}

esp_err_t drive_robot(int linear_speed, int steering_diff) {
    int left_speed = linear_speed - steering_diff;
    int right_speed = linear_speed + steering_diff;

    esp_err_t ret = ESP_OK;

    if (left_speed == 0 && right_speed == 0) {
        ret |= drive_motor(MOTOR_LEFT, STOP, 0);
        ret |= drive_motor(MOTOR_RIGHT, STOP, 0);
        return ret;
    }

    int left_dir = (left_speed >= 0) ? FORWARD : BACKWARD;
    int right_dir = (right_speed >= 0) ? FORWARD : BACKWARD;

    ret |= drive_motor(MOTOR_LEFT, left_dir, left_speed);
    ret |= drive_motor(MOTOR_RIGHT, right_dir, right_speed);
    return ret;
}

