#pragma once
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>

class HealthTracker {
  public:
   HealthTracker(std::atomic<bool>& running_flag,
                 int max_interval_seconds = 20);
   void run_health_loop();
   void update_message_received();
   std::string get_log_timestamp() const;

  private:
   mutable std::mutex health_mutex;
   std::chrono::time_point<std::chrono::steady_clock> last_message_timestamp;
   std::atomic<bool> first_message_received;
   const int robot_message_max_interval;
   std::atomic<bool>& running;
};
