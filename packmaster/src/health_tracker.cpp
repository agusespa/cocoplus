#include "../include/health_tracker.h"

#include <atomic>
#include <chrono>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>

HealthTracker::HealthTracker(std::atomic<bool>& running_flag,
                             int max_interval_seconds)
    : first_message_received(false),
      robot_message_max_interval(max_interval_seconds),
      running(running_flag) {
    last_message_timestamp = std::chrono::steady_clock::now();
}

void HealthTracker::update_message_received() {
    std::lock_guard<std::mutex> lock(health_mutex);
    if (!first_message_received) {
        first_message_received = true;
    }
    last_message_timestamp = std::chrono::steady_clock::now();
}

std::string HealthTracker::get_log_timestamp() const {
    auto now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string timeString(std::ctime(&now));
    if (!timeString.empty() && timeString.back() == '\n') {
        timeString.pop_back();
    }
    return timeString;
}

void HealthTracker::run_health_loop() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        if (!first_message_received) {
            continue;
        }

        std::lock_guard<std::mutex> lock(health_mutex);
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                           now - last_message_timestamp)
                           .count();

        if (elapsed > 20) {
            std::cout << std::endl;
            std::cout << "[" << get_log_timestamp()
                      << "] ALERT: Robot hasn't published data for longer "
                         "that 20 seconds!"
                      << std::endl;
        }
    }
}
