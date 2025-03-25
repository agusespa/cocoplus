#include "../include/robot_view.h"

#include <cmath>
#include <mutex>

const double FOV_DEGREES = 30.0;
const double DEG2RAD = M_PI / 180.0;
const double MAX_DISTANCE = 166.5;

RobotView::RobotView(Controller& controller)
    : controller(controller), window(nullptr), renderer(nullptr) {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Cocoplus", 720, 460, 0);
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

void RobotView::draw_sensor_cone(float cx, float cy, double angle,
                                 double distance) {
    if (distance == 0) {
        distance = MAX_DISTANCE;
    }

    float scale_factor = 3.0f;
    distance *= scale_factor;

    double left_angle = (angle + FOV_DEGREES / 2.0) * DEG2RAD;
    double right_angle = (angle - FOV_DEGREES / 2.0) * DEG2RAD;

    float x1 = cx + distance * cos(left_angle);
    float y1 = cy + distance * sin(left_angle);
    float x2 = cx + distance * cos(right_angle);
    float y2 = cy + distance * sin(right_angle);

    SDL_Color color;
    if (distance < 20 * scale_factor) {
        color = {255, 0, 0, 255};  // Red
    } else if (distance < 40 * scale_factor) {
        color = {255, 165, 0, 255};  // Orange
    } else if (distance < 60 * scale_factor) {
        color = {255, 255, 0, 255};  // Yellow
    } else {
        color = {0, 255, 0, 255};  // Green
    }

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderLine(renderer, cx, cy, x1, y1);
    SDL_RenderLine(renderer, cx, cy, x2, y2);
    SDL_RenderLine(renderer, x1, y1, x2, y2);
}

void RobotView::render() {
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);

    std::lock_guard<std::mutex> lock(controller.get_data_mutex());
    const auto& data = controller.get_current_data();

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    float robot_x = windowWidth / 2.0f;
    float robot_y = windowHeight - 90.f;
    float robot_size = 40.0f;
    SDL_FRect robot_rect = {robot_x - robot_size / 2, robot_y - robot_size / 2,
                            robot_size, robot_size};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &robot_rect);

    draw_sensor_cone(robot_x - robot_size / 2, robot_y, 180, data.left);
    draw_sensor_cone(robot_x, robot_y - robot_size / 2, 270, data.front);
    draw_sensor_cone(robot_x + robot_size / 2, robot_y, 0, data.right);

    SDL_RenderPresent(renderer);
}
