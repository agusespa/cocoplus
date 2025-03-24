#pragma once
#include <mosquitto.h>

#include <functional>
#include <string>
#include <unordered_map>

class MqttClient {
 public:
  using MessageCallback =
      std::function<void(const std::string&, const std::string&)>;
  using TopicHandler = std::function<void(const std::string&)>;

  MqttClient(const std::string& client_id, const std::string& host, int port);
  ~MqttClient();

  bool connect();
  void disconnect();
  void loop_start();
  void loop_stop();

  void publish(const std::string& topic, const std::string& message);
  void subscribe(const std::string& topic);

  void set_message_callback(MessageCallback callback);

  void register_handler(const std::string& topic, TopicHandler handler);
  void remove_handler(const std::string& topic);
  bool has_handler(const std::string& topic) const;

  static const std::string HEALTH_TOPIC;
  static const std::string DATA_TOPIC;
  static const std::string COMMAND_TOPIC;

 private:
  struct mosquitto* mosq;
  std::string host;
  int port;

  MessageCallback message_callback;
  std::unordered_map<std::string, TopicHandler> topic_handlers;
  static void on_message_wrapper(struct mosquitto* mosq, void* userdata,
                                 const struct mosquitto_message* message);
  void process_message(const std::string& topic, const std::string& message);
};
