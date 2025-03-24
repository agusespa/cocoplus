#pragma once
#include <atomic>

#include "data_parser.h"
#include "health_tracker.h"
#include "mqtt_client.h"

class Controller {
  public:
   Controller(MqttClient& mqtt_client);
   void run();

   std::mutex& get_data_mutex() { return data_mutex; }
   DataParser::Data& get_current_data() { return current_data; }
   std::atomic<bool>& get_running() { return running; }

  private:
   DataParser::Data current_data;
   std::mutex data_mutex;

   MqttClient& mqtt_client;
   HealthTracker health_tracker;
   std::atomic<bool> running;

   void setup_mqtt_handlers();
   void run_command_loop();
   void health_handler(const std::string& message);
   void data_handler(const std::string& message);
};

