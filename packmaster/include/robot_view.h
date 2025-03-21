#pragma once
#include <SFML/Graphics.hpp>
#include <atomic>
#include <functional>

class RobotView {
public:
    RobotView(std::atomic<bool>& running_flag);
    void start();
    void set_close_callback(std::function<void()> callback);
    
private:
    sf::RenderWindow window;
    sf::CircleShape robot;
    std::atomic<bool>& running;
    std::function<void()> on_close;
    
    void run_main_loop();
};
