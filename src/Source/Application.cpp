#include "Application.h"

Application::Application()
{
  unsigned int init_flags{
    SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER
  };
  if (init_flags != 0) {
    std::cerr << "Error: \n" << SDL_GetError() << "\n";
    m_exit_status = 1;
  }
}

Application::~Application()
{
  SDL_Quit();
}

int Application::init()
{
  return 0;
}

int Application::render()
{
  return 0;
}

int Application::quit()
{
  return 0;
}

int Application::run()
{
  return 0;
}
