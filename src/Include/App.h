#pragma once

#include <iostream>
#include <memory>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include "Window.h"
#include "texture.h"
#include "glcore.h"
#include "orbiter.h"

class App
{
public:
  App(const int width, const int height, const int major= 3, const int minor= 3, const int samples= 0);
  virtual ~App();

  virtual int init()=0;
  virtual int quit()=0;

  virtual int update(const float time, const float delta) { return 0; }
  virtual int render()=0;

  virtual int run();

private:
  bool event_imgui();

protected:
  virtual int prerender();
  virtual int postrender() { return 0; }

  void vsync_off();

protected:
  bool m_show_ui {true};
  bool m_exit {false};

  bool sync;

  Window m_window; 
  Context m_context;

  Orbiter m_camera;
};
