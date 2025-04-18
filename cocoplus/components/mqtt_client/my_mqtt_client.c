#include "my_mqtt_client.h"

#include <stdio.h>

#include "../../main/cocoplus_main.h"
#include "esp_log.h"

static const char *TAG = "MQTT_CLIENT";

static esp_mqtt_client_handle_t client;
static const char *URI = "mqtt://192.168.1.66:1883";

static const char *COMMAND_TOPIC = "controller/command";

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client_local = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Connected");
            esp_mqtt_client_subscribe(client_local, COMMAND_TOPIC, 0);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Disconnected");
            ESP_LOGI(TAG, "Attempting to reconnect in 5 seconds...");
            vTaskDelay(pdMS_TO_TICKS(5000));
            esp_mqtt_client_start(client);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            if (event->topic != NULL) {
                ESP_LOGI(TAG, "Subscribed to topic: %s", event->topic);
            } else {
                ESP_LOGI(
                    TAG,
                    "Subscribed to topic, topic name not returned in event");
            }
            break;
        case MQTT_EVENT_DATA:
            if (event->topic != NULL && event->data != NULL) {
                ESP_LOGI(TAG, "Received message on topic: %.*s",
                         event->topic_len, event->topic);

                char topic_buf[event->topic_len + 1];
                memcpy(topic_buf, event->topic, event->topic_len);
                topic_buf[event->topic_len] = '\0';

                char message_buf[event->data_len + 1];
                memcpy(message_buf, event->data, event->data_len);
                message_buf[event->data_len] = '\0';

                if (strcmp(topic_buf, COMMAND_TOPIC) == 0) {
                    ESP_LOGI(TAG, "Message data: %s", message_buf);

                    if (strcmp(message_buf, "start") == 0) {
                        if (xSemaphoreTake(controller_mutex, portMAX_DELAY)) {
                            should_stop = false;
                            xSemaphoreGive(controller_mutex);
                        }
                    } else if (strcmp(message_buf, "stop") == 0) {
                        if (xSemaphoreTake(controller_mutex, portMAX_DELAY)) {
                            should_stop = true;
                            xSemaphoreGive(controller_mutex);
                        }
                    }
                }
            } else {
                ESP_LOGE(TAG, "Received message, but topic or data was null");
            }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type ==
                MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGE(TAG, "TCP transport error");
            }
            break;
        default:
            break;
    }
}

esp_err_t mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt5_cfg = {
        .broker.address.uri = URI,
        .session.protocol_ver = MQTT_PROTOCOL_V_5,
        .credentials.username = "",
        .credentials.authentication.password = "",
    };

    client = esp_mqtt_client_init(&mqtt5_cfg);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return ESP_FAIL;
    }

    esp_err_t err = esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID,
                                                   mqtt_event_handler, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register MQTT event handler, error: %d", err);
        return err;
    }

    err = esp_mqtt_client_start(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start MQTT client, error: %d", err);
        return err;
    }

    ESP_LOGI(TAG, "MQTT client started successfully, waiting for connection...");
    return ESP_OK;
}

void mqtt_publish(const char *topic, const char *message) {
    if (client == NULL) {
        ESP_LOGE(TAG, "Cannot publish: MQTT client not initialized");
        return;
    }
    esp_mqtt_client_publish(client, topic, message, 0, 0, 0);
}
