#pragma once
#include <SDL3/SDL.h>
#include <atomic>

class RobotView {
public:
    explicit RobotView(std::atomic<bool>& running_flag);
    ~RobotView();

    void start();

private:
    void render();
    void handle_events();

    std::atomic<bool>& running;
    SDL_Window* window;
    SDL_Renderer* renderer;
};
