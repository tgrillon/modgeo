#pragma once

#include "pch.h"

#include "App.h"
#include "Framebuffer.h"
#include "Bezier.h"
#include "Timer.h"
#include "SDF.h"

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
    int init_demo_patch();
    int init_demo_spline();
    int init_demo_sdf();

    int render_ui();
    int render_demo_buttons();
    int render_stats_patch();
    int render_params_patch();
    int render_stats_spline();
    int render_params_spline();
    int render_stats_sdf();
    int render_params_sdf();
    int render_any();

    int handle_event_spline();
    int handle_event_patch();
    int handle_event_sdf();

    int render_menu_bar();

    void save_spline(const std::string& fullpath) const; 
    void save_patch(const std::string& fullpath) const; 
    void save_sdf(const std::string& fullpath) const; 

    void set_sdf_primitive();
    void set_sdf_operator();
    void build_sdf_tree();
    void render_node_ui(Ref<gm::SDFNode>& node);
    void render_sdf_buttons();

private:
    Ref<Mesh> m_grid;    //! bezier grid
    Ref<Mesh> m_line;    //! bezier curve
    Ref<Mesh> m_mSpline; //! bezier spline mesh
    Ref<Mesh> m_mPatch;  //! bezier patch mesh
    Ref<Mesh> m_mTeapot;
    Ref<Mesh> m_mSDF; //! implicit surface mesh
    Ref<Mesh> m_mSDF_box;

    GLuint m_program_points;
    GLuint m_program_edges;

    Ref<gm::Revolution> m_spline;
    Ref<gm::Bezier> m_patch;

    Ref<gm::Object> m_teapot;

    Framebuffer m_framebuffer;

    std::string m_filename{""}; 

    Ref<gm::SDFTree> m_sdf_tree;
    Ref<gm::SDFNode> m_sdf_node; 
    Ref<gm::SDFNode> m_sdf_root; 

    bool m_node_1_selection{true}; 

    bool m_show_faces_spline{true};
    bool m_show_edges_spline{false};
    bool m_show_points_spline{false};
    bool m_show_faces_patch{true};
    bool m_show_edges_patch{false};
    bool m_show_points_patch{false};
    bool m_show_faces_implicit{true};
    bool m_show_edges_implicit{false};
    bool m_show_points_implicit{false};

    float m_size_point{5.f};
    float m_size_edge{2.f};

    float m_color_point[4]{0.0f, 0.0f, 1.0f, 1.0f};
    float m_color_edge[4]{1.0f, 1.0f, 0.0f, 1.0f};

    float m_smooth_k {0.5};

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
    bool m_sdf_demo{false};

    bool m_show_sdf_box{false};
    bool m_show_spline_curve{false};
    bool m_show_patch_grid{false};

    //! mesh resolutions
    int m_patch_resolution{10};
    int m_spline_resolution{10};
    int m_sdf_resolution{100};
    int m_slide_x{0};
    int m_slide_y{0};
    int m_slide_z{0};

    int m_nb_control_points_patch{10};
    int m_nb_control_points_spline{10};

    bool m_need_update{false};

    int m_spolytms{0}, m_spolytus{0}; //! spline polygonize time
    int m_ppolytms{0}, m_ppolytus{0}; //! patch polygonize time
    int m_ipolytms{0}, m_ipolytus{0}; //! implicit polygonize time

    exprtkWrapper m_expr_spline;
    exprtkWrapper m_expr_patch;

    float pmin[3]{-3.0, -3.0, -3.0};
    float pmax[3]{3.0, 3.0, 3.0};
    gm::Box m_sdf_box{{0.0, 0.0, 0.0}, 5.0};

    Timer m_timer;
};
