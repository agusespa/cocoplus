#include "driver.h"

#include "driver/ledc.h"
#include "esp_log.h"

static const char* TAG = "MOTOR_DRIVER";

#define MOTOR0_PIN1 GPIO_NUM_5   // Left Motor direction pin 1
#define MOTOR0_PIN2 GPIO_NUM_17  // Left Motor direction pin 2
#define MOTOR1_PIN1 GPIO_NUM_14  // Right Motor direction pin 1
#define MOTOR1_PIN2 GPIO_NUM_12  // Right Motor direction pin 2

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
#define PWM_RANGE 635

typedef enum { MOTOR_LEFT, MOTOR_RIGHT } motor_id_t;
typedef enum { FORWARD = 1, BACKWARD = -1, STOP = 0 } motor_direction_t;

static int normalize_duty(float ratio) {
    if (ratio <= 0.0f) {
        return 0;
    } else if (ratio >= 1.0f) {
        return MAX_DUTY;
    } else {
        float pwm_value = (ratio * PWM_RANGE) + MIN_DUTY;
        return (int)pwm_value;
    }
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

void drive_motor(motor_id_t motor, motor_direction_t direction, int speed) {
    ESP_LOGE(TAG, "Speed for motor %d: %d", motor, speed);
    int pin1 =
        (motor == MOTOR_LEFT) ? LEDC_CHANNEL_M0_PIN1 : LEDC_CHANNEL_M1_PIN1;
    int pin2 =
        (motor == MOTOR_LEFT) ? LEDC_CHANNEL_M0_PIN2 : LEDC_CHANNEL_M1_PIN2;

    esp_err_t ret = ESP_OK;

    // TODO improve this mess
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

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error setting motor duty: %d", ret);
    }
}

void drive_robot(float pwm_ratio, int steering_diff) {
    if (pwm_ratio == 0.0) {
        drive_motor(MOTOR_LEFT, STOP, 0);
        drive_motor(MOTOR_RIGHT, STOP, 0);
        return;
    }

    int left_pwm = normalize_duty(pwm_ratio);
    int right_pwm = normalize_duty(pwm_ratio);

    int left_dir = (left_pwm >= 0) ? FORWARD : BACKWARD;
    int right_dir = (right_pwm >= 0) ? FORWARD : BACKWARD;

    drive_motor(MOTOR_LEFT, left_dir, left_pwm);
    drive_motor(MOTOR_RIGHT, right_dir, right_pwm);
}
