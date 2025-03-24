#include "../include/robot_view.h"

#include <iostream>
#include <mutex>
#include <ostream>

RobotView::RobotView(Controller& controller)
    : controller(controller), window(nullptr), renderer(nullptr) {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Cocoplus", 800, 800, 0);
    renderer = SDL_CreateRenderer(window, NULL);
}

RobotView::~RobotView() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

void RobotView::start() {
    while (controller.get_running()) {
        handle_events();
        render();
        SDL_Delay(16);
    }
}

void RobotView::handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
    }
}

void RobotView::draw_data(const DataParser::Data& data) {
    std::cout << "Successfully parsed message:\n"
              << "  Left speed: " << data.left << "\n"
              << "  Front sensor: " << data.front << "\n"
              << "  Right sensor: " << data.right << std::endl;
}

void RobotView::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_FPoint points[4] = {
        {400.f, 380.f}, {380.f, 420.f}, {420.f, 420.f}, {400.f, 380.f}};
    SDL_RenderLines(renderer, points, 4);

    std::lock_guard<std::mutex> lock(controller.get_data_mutex());
    const auto& data = controller.get_current_data();
    draw_data(data);

    SDL_RenderPresent(renderer);
}
