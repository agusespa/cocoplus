#pragma once
#include "health_tracker.h"
#include "mqtt_client.h"
#include "robot_view.h"
#include <atomic>
#include <thread>

class Controller {
public:
    Controller(MqttClient& mqtt_client);
    void run();
    
private:
    MqttClient& mqtt_client;
    HealthTracker health_tracker;
    std::atomic<bool> running;

    void setup_mqtt_handlers();
    void run_command_loop();
    void health_handler(const std::string& message);
    void data_handler(const std::string& message);
};
