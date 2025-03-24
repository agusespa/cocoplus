#include "../include/controller.h"

#include <iostream>
#include <string>

#include "../include/health_tracker.h"

Controller::Controller(MqttClient& mqtt_client)
    : mqtt_client(mqtt_client), running(true), health_tracker(running, 20) {}

void Controller::run() {
    setup_mqtt_handlers();

    mqtt_client.subscribe(MqttClient::HEALTH_TOPIC);
    mqtt_client.subscribe(MqttClient::DATA_TOPIC);
    mqtt_client.loop_start();

    RobotView robot_view(running);

    std::thread command_thread(&Controller::run_command_loop, this);

    std::thread health_thread(&HealthTracker::run_health_loop, &health_tracker);

    robot_view.start();

    command_thread.join();
    health_thread.join();
}

void Controller::setup_mqtt_handlers() {
    mqtt_client.register_handler(
        MqttClient::HEALTH_TOPIC,
        [this](const std::string& message) { health_handler(message); });

    mqtt_client.register_handler(
        MqttClient::DATA_TOPIC,
        [this](const std::string& message) { data_handler(message); });
}

void Controller::run_command_loop() {
    std::cout << "Available commands: start, stop, shutdown" << std::endl;

    std::string command;
    while (running) {
        std::cout << "> ";
        std::getline(std::cin, command);

        if (command == "start" || command == "stop" || command == "shutdown") {
            mqtt_client.publish(MqttClient::COMMAND_TOPIC, command);
            if (command == "shutdown") {
                std::cout << "Shutting down..." << std::endl;
                running = false;
                break;
            }
        } else {
            std::cout << "Unknown command." << std::endl;
        }
    }
}

void Controller::health_handler(const std::string& message) {
    health_tracker.update_message_received();
    if (message.find("error") != std::string::npos) {
        std::cout << "[" << health_tracker.get_log_timestamp()
                  << "] ALERT: Robot reported an error!" << std::endl;
    }
}

void Controller::data_handler(const std::string& message) {
    health_tracker.update_message_received();

    auto parsed_data = DataParser::parse(message);
    std::cout << "Successfully parsed message:\n"
              << "Left: " << parsed_data.left << "\n"
              << "Front: " << parsed_data.front << "\n"
              << "Right: " << parsed_data.right << std::endl;
}
