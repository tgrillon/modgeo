#pragma once

#include "pch.h"

#include "App.h"
#include "Framebuffer.h"
#include "bezier.h"
#include "utils.h"

class Viewer : public App
{
public:
  Viewer();

  int init_any() override;
  int init_imgui() override;

  int render() override;

  int quit_any() override;
  int quit_imgui() override;

private:
  int render_ui();
  int render_any();

  int render_menu_bar(); 

private: 
  Mesh m_grid;
  Mesh m_mSpline;
  Mesh m_mPatch;
  Mesh m_line;

  gm::Revolution m_spline;
  gm::Bezier m_patch;

  Framebuffer m_framebuffer; 

  bool m_spline_edges_only {false};
  bool m_spline_points_only {false};
  bool m_patch_edges_only {false};
  bool m_patch_points_only {false};

  char spline_radial_function_input[256] {"((sin(110 * t) + 1) * 0.5 + 1) * max(1.5pi, a)"};
  char curve_function_input_x[256] {"100 * t"};
  char curve_function_input_y[256] {"25 * sin(2t) * cos(2t)"};
  char curve_function_input_z[256] {"0"};     
  char surface_function_input_x[256] {"u * 10."};
  char surface_function_input_y[256] {"sin(u * 2 * 3.14159) * cos(v * 2 * 3.14159)"};
  char surface_function_input_z[256] {"v * 10."};

  bool m_show_style_editor{false};
  bool m_show_ui {true};
  bool m_dark_theme {true};

  bool m_show_spline {true}; 
  bool m_show_spline_curve {false}; 
  bool m_show_patch {false};
  bool m_show_patch_grid {false};

  int m_patch_resolution {10}; 
  int m_surface_degree {10};
  int m_curve_degree {10};
  int m_spline_resolution {10}; 
  bool m_need_update{false}; 

  double m_spline_time_polygonize {0};
  double m_patch_time_polygonize {0};

  exprtkWrapper m_expr_spline; 
  exprtkWrapper m_expr_patch; 
};