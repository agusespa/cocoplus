#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <mosquitto.h>
#include <string>
#include <functional>

class MqttClient {
public:
    MqttClient(const std::string &client_id, const std::string &host, int port);
    ~MqttClient();

    bool connect();
    void disconnect();
    void publish(const std::string &topic, const std::string &message);
    void subscribe(const std::string &topic);
    
    void loop_start();
    void loop_stop();

    void set_message_callback(std::function<void(const std::string&, const std::string&)> callback);

private:
    static void on_message_wrapper(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);
    
    struct mosquitto *mosq;
    std::string host;
    int port;
    std::function<void(const std::string&, const std::string&)> message_callback;
};

#endif // MQTT_CLIENT_H
