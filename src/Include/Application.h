#pragma once

#include <iostream>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include <SDL2/SDL.h>

class Application
{
public:
 Application();
  ~Application();

  int init();
  int render();
  int quit();

  int run();
private:
  int m_exit_status {0};
  bool m_running {true};
};
