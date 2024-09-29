#pragma once

#include "App.h"
#include "draw.h"
#include "mesh.h"

#include "Framebuffer.h"
#include "Bezier.h"

class Viewer : public App
{
public:
  Viewer();

  int init_any();
  int quit_any();

  int render();

private:
  void render_ui();
  void init_menu_bar(); 

private: 
  Mesh m_grid;
  Mesh m_patch;

  mg::Bezier m_bezier;
  mg::Grid m_hm;

  Framebuffer m_framebuffer; 

  bool m_show_style_editor{false};

  int m_resolution{10}; 
};