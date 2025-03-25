#pragma once
#include <SDL3/SDL.h>
#include <data_parser.h>

#include "controller.h"

class RobotView {
 public:
  RobotView(Controller& controller);
  ~RobotView();

  void start();

 private:
  void handle_events();
  void render();
  void draw_data(const DataParser::Data& data);
  void draw_sensor_cone(float cx, float cy, double angle, double distance);

  Controller& controller;

  SDL_Window* window;
  SDL_Renderer* renderer;
};
