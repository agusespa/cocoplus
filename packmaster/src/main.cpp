#include <iostream>
#include <string>

#include "../include/mqtt_client.h"

void message_handler(const std::string& topic, const std::string& message) {
    std::cout << "Received on [" << topic << "]: " << message << std::endl;
}

int main() {
    MqttClient mqtt_client("robot_controller", "localhost", 1883);

    if (!mqtt_client.connect()) {
        std::cerr << "Failed to connect" << std::endl;
        return 1;
    }

    mqtt_client.set_message_callback(message_handler);

    mqtt_client.subscribe("robot/commands");

    mqtt_client.loop_start();

    std::string command;
    while (true) {
        std::cout << "Enter command (start/stop/exit): ";
        std::cin >> command;

        if (command == "exit") break;

        mqtt_client.publish("robot/command", command);
    }

    mqtt_client.loop_stop();
    mqtt_client.disconnect();

    return 0;
}
