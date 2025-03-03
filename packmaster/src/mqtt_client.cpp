#include "../include/mqtt_client.h"

#include <iostream>

MqttClient::MqttClient(const std::string &client_id, const std::string &host,
                       int port)
    : host(host), port(port), mosq(nullptr) {
    mosquitto_lib_init();
    mosq = mosquitto_new(client_id.c_str(), true, this);

    if (!mosq)
        throw std::runtime_error("Failed to create Mosquitto client instance.");

    mosquitto_message_callback_set(mosq, MqttClient::on_message_wrapper);
}

MqttClient::~MqttClient() {
    if (mosq) mosquitto_destroy(mosq);

    mosquitto_lib_cleanup();
}

bool MqttClient::connect() {
    int result = mosquitto_connect(mosq, host.c_str(), port, 60);

    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect to broker: "
                  << mosquitto_strerror(result) << std::endl;
        return false;
    }

    return true;
}

void MqttClient::disconnect() { mosquitto_disconnect(mosq); }

void MqttClient::loop_start() { mosquitto_loop_start(mosq); }

void MqttClient::loop_stop() { mosquitto_loop_stop(mosq, true); }

void MqttClient::publish(const std::string &topic, const std::string &message) {
    int result = mosquitto_publish(mosq, nullptr, topic.c_str(), message.size(),
                                   message.c_str(), 0, false);

    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to publish message: " << mosquitto_strerror(result)
                  << std::endl;
    }
}

void MqttClient::subscribe(const std::string &topic) {
    int result = mosquitto_subscribe(mosq, nullptr, topic.c_str(), 0);

    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to subscribe to topic: "
                  << mosquitto_strerror(result) << std::endl;
    }
}

void MqttClient::set_message_callback(
    std::function<void(const std::string &, const std::string &)> callback) {
    message_callback = callback;
}

void MqttClient::on_message_wrapper(struct mosquitto *mosq, void *userdata,
                                    const struct mosquitto_message *message) {
    auto *client = static_cast<MqttClient *>(userdata);
    if (client->message_callback) {
        client->message_callback(
            message->topic, std::string(static_cast<char *>(message->payload),
                                        message->payloadlen));
    }
}
