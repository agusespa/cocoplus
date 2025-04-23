#include "driver.h"

#include "esp_log.h"

static const char* TAG = "MOTOR_DRIVER";

esp_err_t motor_init() {
    gpio_config_t motor1_pins = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << MOTOR0_PIN1) | (1ULL << MOTOR0_PIN2)};
    esp_err_t err1 = gpio_config(&motor1_pins);
    if (err1 != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure Left Motor pins: %d", err1);
        return err1;
    }
    gpio_set_level(MOTOR0_PIN1, 0);
    gpio_set_level(MOTOR0_PIN2, 0);
    ESP_LOGI(TAG, "Left Motor pins initialized to STOP.");

    gpio_config_t motor2_pins = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << MOTOR1_PIN1) | (1ULL << MOTOR1_PIN2)};
    esp_err_t err2 = gpio_config(&motor2_pins);
    if (err2 != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure Right Motor pins: %d", err2);
        return err2;
    }
    gpio_set_level(MOTOR1_PIN1, 0);
    gpio_set_level(MOTOR1_PIN2, 0);
    ESP_LOGI(TAG, "Right Motor pins initialized to STOP.");

    return ESP_OK;
}

void set_motor_direction(motor_id_t motor_id, motor_direction_t direction) {
    gpio_num_t pin1, pin2;

    if (motor_id == MOTOR_LEFT) {
        pin1 = MOTOR0_PIN1;
        pin2 = MOTOR0_PIN2;
    } else if (motor_id == MOTOR_RIGHT) {
        pin1 = MOTOR1_PIN1;
        pin2 = MOTOR1_PIN2;
    } else {
        ESP_LOGE(TAG, "Invalid motor ID: %d", motor_id);
        return;
    }

    if (direction == MOTOR_FORWARD) {
        gpio_set_level(pin1, 1);
        gpio_set_level(pin2, 0);
    } else if (direction == MOTOR_BACKWARD) {
        gpio_set_level(pin1, 0);
        gpio_set_level(pin2, 1);
    } else if (direction == MOTOR_STOP) {
        gpio_set_level(pin1, 0);
        gpio_set_level(pin2, 0);
    } else {
        ESP_LOGE(TAG, "Invalid direction for motor %d: %d", motor_id,
                 direction);
    }
}
