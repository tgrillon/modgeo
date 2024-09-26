#pragma once

#include "App.h"
#include "draw.h"
#include "mesh.h"

#include "Bezier.h"

class Viewer : public App
{
public:
  Viewer();

  int init();
  int quit();

  int render();

private:
  int init_sdl_gl();
  int init_imgui();

  void render_ui();
  void init_control_panel(); 

private: 
  Mesh m_grid;
  Mesh m_patch;

  mg::Bezier m_bezier;

  int m_resolution{10}; 

  ImGuiIO io;
};