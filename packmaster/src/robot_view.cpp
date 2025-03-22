#include "../include/robot_view.h"

RobotView::RobotView(std::atomic<bool>& running_flag)
    : running(running_flag), window(nullptr), renderer(nullptr) {
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
    while (running) {
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

void RobotView::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_FPoint points[4] = {
        {400.f, 380.f}, {380.f, 420.f}, {420.f, 420.f}, {400.f, 380.f}};
    SDL_RenderLines(renderer, points, 4);
    SDL_RenderPresent(renderer);
}
