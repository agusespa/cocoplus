#pragma once
#include "mqtt_client.h"

class Monitor {
public:
    Monitor(MqttClient& mqtt_client);
    void run();
    
private:
    MqttClient& mqtt_client;
    
    void message_handler(const std::string& topic, const std::string& message);
};
