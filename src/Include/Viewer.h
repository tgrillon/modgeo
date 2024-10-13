#pragma once

#include "pch.h"

#include "App.h"
#include "Framebuffer.h"
#include "bezier.h"
#include "Timer.h"

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
    Mesh m_mTeapot;

    gm::Revolution m_spline;
    gm::Bezier m_patch;

    gm::Object m_teapot;

    Framebuffer m_framebuffer;

    bool m_spline_edges_only{false};
    bool m_spline_points_only{false};
    bool m_patch_edges_only{false};
    bool m_patch_points_only{false};

    char spline_radial_function_input[256]{"1"};
    char curve_function_input_x[256]{"100 * t"};
    char curve_function_input_y[256]{"25 * sin(2t) * cos(2t)"};
    char curve_function_input_z[256]{"0"};
    char surface_function_input_x[256]{"u * 10."};
    char surface_function_input_y[256]{"sin(u * 2 * 3.14159) * cos(v * 2 * 3.14159)"};
    char surface_function_input_z[256]{"v * 10."};

    bool m_show_style_editor{false};
    bool m_show_ui{true};
    bool m_dark_theme{true};

    bool m_show_spline{true};
    bool m_show_spline_curve{false};
    bool m_show_patch{false};
    bool m_show_patch_grid{false};

    int m_patch_resolution{10};
    int m_nb_control_points_patch{10};
    int m_nb_control_points_spline{10};
    int m_spline_resolution{10};
    bool m_need_update{false};

    int m_spolytms{0}, m_spolytus{0}; // spline polygonize time
    int m_ppolytms{0}, m_ppolytus{0}; // patch polygonize time

    exprtkWrapper m_expr_spline;
    exprtkWrapper m_expr_patch;

    Timer m_timer;
};
