#include "../include/robot_view.h"

RobotView::RobotView(std::atomic<bool>& running_flag) : running(running_flag) {}

void RobotView::start() { run_main_loop(); }

void RobotView::set_close_callback(std::function<void()> callback) {
    on_close = callback;
}

void RobotView::run_main_loop() {
    sf::VideoMode videoMode({800, 800}, 24);
    sf::RenderWindow window(videoMode, "Cocoplus");
    window.setPosition(sf::Vector2i(100, 100));

    sf::CircleShape robot(20.f, 3);
    robot.setPosition({380.f, 380.f});
    robot.setFillColor(sf::Color::White);

    while (window.isOpen() && running) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                on_close();
            } else if (const auto* keyPressed =
                           event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    window.close();
                    on_close();
                }
            }
        }
        window.clear(sf::Color::Black);
        window.draw(robot);
        window.display();
    }
}
