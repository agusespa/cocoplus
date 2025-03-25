#include "../include/robot_view.h"

#include <mutex>
#include <cmath>

const double FOV_DEGREES = 30.0;
const double DEG2RAD = M_PI / 180.0;
const double MAX_DISTANCE = 166.5;

RobotView::RobotView(Controller& controller)
    : controller(controller), window(nullptr), renderer(nullptr) {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Cocoplus", 720, 440, 0);
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
        SDL_Delay(250);
    }
}

void RobotView::handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
    }
}

void RobotView::draw_sensor_cone(float cx, float cy, double angle, double distance) {
    if (distance == 0) {
        distance = MAX_DISTANCE;
    }

    double left_angle = (angle + FOV_DEGREES / 2.0) * DEG2RAD;
    double right_angle = (angle - FOV_DEGREES / 2.0) * DEG2RAD;

    float x1 = cx + distance * cos(left_angle);
    float y1 = cy + distance * sin(left_angle);
    float x2 = cx + distance * cos(right_angle);
    float y2 = cy + distance * sin(right_angle);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 100);
    SDL_Vertex v[3] = {
        {cx, cy, {255, 0, 0, 100}},
        {x1, y1, {255, 0, 0, 100}},
        {x2, y2, {255, 0, 0, 100}}
    };
    SDL_RenderGeometry(renderer, NULL, v, 3, NULL, 0);
}

void RobotView::render() {
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);

    std::lock_guard<std::mutex> lock(controller.get_data_mutex());
    const auto& data = controller.get_current_data();

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    float robot_x = windowWidth / 2.0f;
    float robot_y = windowHeight - 80.f;
    float robot_size = 40.0f;
    SDL_FRect robot_rect = {robot_x - robot_size / 2, robot_y - robot_size / 2, robot_size, robot_size};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &robot_rect);

    draw_sensor_cone(robot_x - robot_size / 2, robot_y, 180, data.left);
    draw_sensor_cone(robot_x, robot_y - robot_size / 2, 270, data.front);
    draw_sensor_cone(robot_x + robot_size / 2, robot_y, 0, data.right);

    SDL_RenderPresent(renderer);
}
