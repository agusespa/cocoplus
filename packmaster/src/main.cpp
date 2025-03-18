#include <atomic>
#include <chrono>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>

#include "../include/mqtt_client.h"

enum class AppMode { MONITOR, CONTROLLER };

std::mutex health_mutex;
std::string last_health_message;
std::chrono::time_point<std::chrono::steady_clock> last_health_timestamp;
std::atomic<bool> first_message_received(false);

std::string get_timestamp() {
    auto now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    return std::ctime(&now);
}

std::string get_log_timestamp() {
    auto now = get_timestamp();

    std::string timeString(now);
    if (!timeString.empty() && timeString.back() == '\n') {
        timeString.pop_back();
    }
    return timeString;
}

void controller_health_handler(const std::string& message) {
    std::lock_guard<std::mutex> lock(health_mutex);

    last_health_message = message;
    last_health_timestamp = std::chrono::steady_clock::now();

    if (!first_message_received) {
        first_message_received = true;
    }

    if (message.find("error") != std::string::npos) {
        std::cout << std::endl;
        std::cout << "[" << get_log_timestamp()
                  << "] ALERT: Robot reported an error condition!" << std::endl;
    }
}

void controller_data_handler(const std::string& message) {
    // TODO pending implementation
}

void monitor_message_handler(const std::string& topic,
                             const std::string& message) {
    std::cout << "[" << get_log_timestamp() << "] RECV: " << topic << " - "
              << message << std::endl;
}

void run_monitor_mode(MqttClient& mqtt_client) {
    std::cout << "Listening... Press Enter to exit" << std::endl;

    mqtt_client.set_message_callback(monitor_message_handler);

    mqtt_client.subscribe("cocoplus/health");
    mqtt_client.subscribe("cocoplus/data");

    mqtt_client.loop_start();

    std::cin.get();
}

std::atomic<bool> running(true);

void health_monitor() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        if (!first_message_received) {
            continue;
        }

        std::lock_guard<std::mutex> lock(health_mutex);
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                           now - last_health_timestamp)
                           .count();

        if (elapsed > 20) {
            std::cout << std::endl;
            std::cout << "[" << get_log_timestamp()
                      << "] ALERT: Robot hasn't reported vitals for longer "
                         "that 20 seconds!"
                      << std::endl;
        }
    }
}

void run_controller_mode(MqttClient& mqtt_client) {
    std::cout << "Available commands:" << std::endl;
    std::cout << "  start    - Start the robot" << std::endl;
    std::cout << "  stop     - Stop the robot" << std::endl;
    std::cout << "  shutdown - Shutdown the robot and exit" << std::endl;

    mqtt_client.register_handler("cocoplus/health", controller_health_handler);
    mqtt_client.register_handler("cocoplus/data", controller_data_handler);

    mqtt_client.subscribe("cocoplus/health");
    mqtt_client.subscribe("cocoplus/data");

    mqtt_client.loop_start();

    std::thread health_thread(health_monitor);

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
                running = false;
                health_thread.join();
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
