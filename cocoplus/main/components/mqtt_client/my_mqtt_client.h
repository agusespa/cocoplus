#ifndef MY_MQTT_CLIENT_H
#define MY_MQTT_CLIENT_H

#include "mqtt_client.h"

esp_err_t mqtt_app_start(void);
void mqtt_publish(const char *topic, const char *message);

#endif  // MY_MQTT_CLIENT_H
