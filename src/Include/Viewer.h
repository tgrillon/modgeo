#pragma once

#include "pch.h"

#include "App.h"
#include "Framebuffer.h"
#include "Bezier.h"
#include "Timer.h"
#include "Implicit.h"

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
    int render_demo_buttons();
    int render_patch_stats();
    int render_patch_params();
    int render_spline_stats();
    int render_spline_params();
    int render_implicit_stats();
    int render_implicit_params();
    int render_any();

    int render_menu_bar();

private:
    Mesh m_grid; //! bezier grid 
    Mesh m_line; //! bezier curve
    Mesh m_mSpline; //! bezier spline mesh 
    Mesh m_mPatch; //! bezier patch mesh
    Mesh m_mTeapot;
    Mesh m_mImplicit; //! implicit surface mesh 

    gm::Revolution m_spline;
    gm::Bezier m_patch;

    gm::Object m_teapot;

    Framebuffer m_framebuffer;

    Ref<gm::ImplicitTree> m_imp_tree; 

    bool m_spline_edges_only{false};
    bool m_spline_points_only{false};
    bool m_patch_edges_only{false};
    bool m_patch_points_only{false};
    bool m_implicit_edges_only{false};
    bool m_implicit_points_only{false};

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

    //! Demo 
    bool m_spline_demo{true};
    bool m_patch_demo{false};
    bool m_implicit_demo{false};

    bool m_show_spline_curve{false};
    bool m_show_patch_grid{false};

    //! mesh resolutions
    int m_patch_resolution{10};
    int m_spline_resolution{10};
    int m_implicit_resolution{10};

    int m_nb_control_points_patch{10};
    int m_nb_control_points_spline{10};

    bool m_need_update{false};

    int m_spolytms{0}, m_spolytus{0}; //! spline polygonize time
    int m_ppolytms{0}, m_ppolytus{0}; //! patch polygonize time
    int m_ipolytms{0}, m_ipolytus{0}; //! implicit polygonize time

    exprtkWrapper m_expr_spline;
    exprtkWrapper m_expr_patch;

    gm::Box m_imp_box {{-3.0, -3.0, -3.0}, {3.0, 3.0, 3.0}}; 

    Timer m_timer;
};
