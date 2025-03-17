#include <ctime>
#include <iostream>
#include <string>

#include "../include/mqtt_client.h"

enum class AppMode { MONITOR, CONTROLLER };

std::string get_timestamp() {
    std::time_t now = std::time(nullptr);
    char* dt = std::ctime(&now);

    if (dt == nullptr) {
        return "Error getting time";
    }

    std::string timeString(dt);
    if (!timeString.empty() && timeString.back() == '\n') {
        timeString.pop_back();
    }
    return timeString;
}

void controller_health_handler(const std::string& message) {
    if (message.find("error") != std::string::npos) {
        std::cout << std::endl;
        std::cout << "[" << get_timestamp()
                  << "] ALERT: Robot reported an error condition!" << std::endl;
    }
}

void monitor_message_handler(const std::string& topic,
                             const std::string& message) {
    std::cout << "[" << get_timestamp() << "] RECV: " << topic << " - "
              << message << std::endl;
}

void run_monitor_mode(MqttClient& mqtt_client) {
    std::cout << "=== MQTT Monitor Mode ===" << std::endl;
    std::cout << "Listening... Press Enter to exit" << std::endl;

    mqtt_client.set_message_callback(monitor_message_handler);

    mqtt_client.subscribe("cocoplus/data");
    mqtt_client.subscribe("cocoplus/health");

    mqtt_client.loop_start();

    std::cin.get();
}

void run_controller_mode(MqttClient& mqtt_client) {
    std::cout << "=== MQTT Controller Mode ===" << std::endl;
    std::cout << "Available commands:" << std::endl;
    std::cout << "  start    - Start the robot" << std::endl;
    std::cout << "  stop     - Stop the robot" << std::endl;
    std::cout << "  shutdown - Shutdown robot and exit" << std::endl;

    mqtt_client.register_handler("cocoplus/health", controller_health_handler);
    mqtt_client.subscribe("cocoplus/health");

    mqtt_client.loop_start();

    std::string command;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, command);

        if (command.empty()) {
            continue;
        }

        if (command == "start" || command == "stop" || command == "shutdown") {
            mqtt_client.publish("controller/command", command);

            if (command == "shutdown") {
                std::cout << "Shutting down and exiting..." << std::endl;
                break;
            }
        } else {
            std::cout << "Unknown command. Please try again." << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    AppMode mode = AppMode::CONTROLLER;

    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "-m") {
            mode = AppMode::MONITOR;
        } else if (arg == "-c") {
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

    MqttClient mqtt_client("robot_controller", "localhost", 1883);

    std::cout << "Connecting to MQTT broker..." << std::endl;
    if (!mqtt_client.connect()) {
        std::cerr << "Failed to connect to MQTT broker" << std::endl;
        return 1;
    }
    std::cout << "Connected to MQTT broker" << std::endl;

    if (mode == AppMode::MONITOR) {
        run_monitor_mode(mqtt_client);
    } else {
        run_controller_mode(mqtt_client);
    }

    mqtt_client.loop_stop();
    mqtt_client.disconnect();

    return 0;
}
