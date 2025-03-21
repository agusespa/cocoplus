#include "../include/monitor.h"

#include <ctime>
#include <iostream>

Monitor::Monitor(MqttClient& mqtt_client) : mqtt_client(mqtt_client) {}

void Monitor::run() {
    std::cout << "Listening... Press Enter to exit" << std::endl;

    mqtt_client.set_message_callback(
        [this](const std::string& topic, const std::string& message) {
            message_handler(topic, message);
        });

    mqtt_client.subscribe("cocoplus/health");
    mqtt_client.subscribe("cocoplus/data");
    mqtt_client.loop_start();

    std::cin.get(); // Terminate on Enter
}

void Monitor::message_handler(const std::string& topic,
                              const std::string& message) {
    auto now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string timeString(std::ctime(&now));
    if (!timeString.empty() && timeString.back() == '\n') {
        timeString.pop_back();
    }

    std::cout << "[" << timeString << "] RECV: " << topic << " - " << message
              << std::endl;
}
