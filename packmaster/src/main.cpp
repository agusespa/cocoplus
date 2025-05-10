#include <iostream>
#include <string>

#include "../include/controller.h"
#include "../include/monitor.h"
#include "../include/mqtt_client.h"

enum class AppMode { MONITOR, CONTROLLER };

int main(int argc, char* argv[]) {
    AppMode mode = AppMode::CONTROLLER;

    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "-m") {
            std::cout << "=== MQTT Monitor Mode ===" << std::endl;
            mode = AppMode::MONITOR;
        } else if (arg == "-c") {
            std::cout << "=== MQTT Controller Mode ===" << std::endl;
            mode = AppMode::CONTROLLER;
        } else {
            std::cout << "Unknown argument: " << arg << std::endl;
            std::cout << "Usage: " << argv[0] << " [-m|-c]" << std::endl;
            return 1;
        }
    } else {
        std::cout << "Please provide the mode flag: [-m|-c]" << std::endl;
        return 1;
    }

    MqttClient mqtt_client("robot_controller", "192.168.1.120", 1883);

    std::cout << "Connecting to MQTT broker..." << std::endl;
    if (!mqtt_client.connect()) {
        std::cerr << "Failed to connect to MQTT broker" << std::endl;
        return 1;
    }
    std::cout << "Connected to MQTT broker" << std::endl;

    if (mode == AppMode::MONITOR) {
        Monitor monitor(mqtt_client);
        monitor.run();
    } else {
        Controller controller(mqtt_client);
        controller.run();
    }

    mqtt_client.loop_stop();
    mqtt_client.disconnect();

    return 0;
}
