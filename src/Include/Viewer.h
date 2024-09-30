#pragma once

#include "draw.h"
#include "mesh.h"

#include "App.h"
#include "Framebuffer.h"
#include "Bezier.h"
#include "utils.h"

class Viewer : public App
{
public:
  Viewer();

  int init_any();
  int quit_any();

  int render();

private:
  int render_ui();
  int render_any();

  int render_menu_bar(); 

private: 
  Mesh m_grid;
  Mesh m_patch;

  mg::Bezier m_bezier;
  mg::Grid m_hm;

  Framebuffer m_framebuffer; 

  bool m_show_style_editor{false};
  bool m_show_ui {true};

  int m_resolution{10}; 
  bool m_need_update{false}; 
};