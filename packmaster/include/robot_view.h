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

  Controller& controller;

  SDL_Window* window;
  SDL_Renderer* renderer;
};
