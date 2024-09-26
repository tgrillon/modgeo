#include "Window.h"

Window::Window(const Settings& settings) {

  const auto window_flags{
      static_cast<SDL_WindowFlags>(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI)};

  m_window = SDL_CreateWindow(settings.title.c_str(),
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      settings.width,
      settings.height,
      window_flags);

  Uint32 renderer_flags{SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED};
  m_renderer = SDL_CreateRenderer(m_window, -1, renderer_flags);

  if (m_renderer == nullptr) {
    std::cerr << "Error creating SDL_Renderer!" << "\n";
    return;
  }

  SDL_RendererInfo info;
  SDL_GetRendererInfo(m_renderer, &info);
}

Window::~Window() 
{
  SDL_DestroyRenderer(m_renderer);
  SDL_DestroyWindow(m_window);
}

SDL_Window* Window::get_native_window() const 
{
  return m_window;
}

SDL_Renderer* Window::get_native_renderer() const 
{
  return m_renderer;
}
