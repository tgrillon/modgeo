#pragma once

#include <string>
#include <iostream>

#include <SDL2/SDL.h>

class Window {
 public:
  struct Settings {
    std::string title;
    int width{1280};
    int height{720};
  };

  explicit Window(const Settings& settings);
  ~Window();

  Window(const Window&) = delete;
  Window(Window&&) = delete;
  Window& operator=(Window other) = delete;
  Window& operator=(Window&& other) = delete;

  SDL_Window* get_native_window() const;
  SDL_Renderer* get_native_renderer() const;

 private:
  SDL_Window* m_window{nullptr};
  SDL_Renderer* m_renderer{nullptr};
};
