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
  Mesh m_mSpline;
  Mesh m_mPatch;

  gm::Spline m_spline;
  gm::Patch m_patch;

  Framebuffer m_framebuffer; 

  bool m_spline_edges_only {false};
  bool m_patch_edges_only {false};

  char spline_radial_function_input[256] {"1.0"};
  char spline_function_input_x[256] {"10 * cos(t * 2 * 3.14159)"};
  char spline_function_input_y[256] {"10 * sin(t * 2 * 3.14159)"};
  char spline_function_input_z[256] {"10 * t * 2 * 3.14159"};     
  char surface_function_input_x[256] {"u * 10."};
  char surface_function_input_y[256] {"sin(u * 2 * 3.14159) * cos(v * 2 * 3.14159)"};
  char surface_function_input_z[256] {"v * 10."};

  bool m_show_style_editor{false};
  bool m_show_ui {true};
  bool m_dark_theme {true};

  bool m_show_spline {true}; 
  bool m_show_patch {false};

  int m_patch_resolution{10}; 
  int m_spline_resolution{10}; 
  bool m_need_update{false}; 

  double m_spline_time_polygonize {0};
  double m_patch_time_polygonize {0};
};