#include "Viewer.h"

#include "utils.h"

Mesh make_grid(const int n = 10)
{
    Mesh grid = Mesh(GL_LINES);

    // grille
    grid.color(White());
    for (int x = 0; x < n; x++)
    {
        float px = float(x) - float(n) / 2 + .5f;
        grid.vertex(Point(px, 0, -float(n) / 2 + .5f));
        grid.vertex(Point(px, 0, float(n) / 2 - .5f));
    }

    for (int z = 0; z < n; z++)
    {
        float pz = float(z) - float(n) / 2 + .5f;
        grid.vertex(Point(-float(n) / 2 + .5f, 0, pz));
        grid.vertex(Point(float(n) / 2 - .5f, 0, pz));
    }

    // axes XYZ
    grid.color(Red());
    grid.vertex(Point(0, .1, 0));
    grid.vertex(Point(1, .1, 0));

    grid.color(Green());
    grid.vertex(Point(0, .1, 0));
    grid.vertex(Point(0, 1, 0));

    grid.color(Blue());
    grid.vertex(Point(0, .1, 0));
    grid.vertex(Point(0, .1, 1));

    glLineWidth(2);

    return grid;
}

GLuint read_program(const std::string &filepath)
{
    return read_program(filepath.c_str());
}

Viewer::Viewer() : App(1024, 640), m_framebuffer(window_width(), window_height())
{
}

int Viewer::init_any()
{
    m_program_points = read_program(std::string(SHADER_DIR) + "/points.glsl");
    program_print_errors(m_program_points);

    m_program_edges = read_program(std::string(SHADER_DIR) + "/edges.glsl");
    program_print_errors(m_program_edges);

    //! Bezier spline intialization
    std::vector<Point> curve;

    curve = gm::curve_points(m_nb_control_points_spline, [](double t)
                             { return Point(10 * t, 0, 0); });

    m_line = create_ref<Mesh>(GL_LINE_STRIP);
    for (const auto &p : curve)
    {
        m_line->vertex(p);
    }

    m_spline = gm::Revolution::create(curve);

    m_timer.start();
    m_mSpline = m_spline->polygonize(4);
    m_timer.stop();

    m_spolytms = m_timer.ms();
    m_spolytus = m_timer.us();

    if (m_patch_demo)
    {
        center_camera(*m_mPatch);
    }

    //! Bezier patch intialization
    std::vector<std::vector<Point>> surface;

    surface = gm::surface_points(m_nb_control_points_patch, [](double u, double v)
                                 { return Point(u * 10., sin(10 * v * 2 * M_PI), v * 10); });

    m_grid = create_ref<Mesh>(GL_LINES);
    for (int h = 1; h < m_nb_control_points_patch; ++h)
    {
        for (int w = 1; w < m_nb_control_points_patch; ++w)
        {
            m_grid->vertex(surface[h][w - 1]);
            m_grid->vertex(surface[h][w]);

            m_grid->vertex(surface[h - 1][w]);
            m_grid->vertex(surface[h][w]);
        }
    }

    m_patch = gm::Bezier::create(surface);

    m_timer.start();
    m_mPatch = m_patch->polygonize(m_patch_resolution);
    m_timer.stop();

    m_ppolytms = m_timer.ms();
    m_ppolytus = m_timer.us();

    if (m_spline_demo)
    {
        center_camera(*m_mSpline);
    }

    //! Implicit surface initialization
    Ref<gm::ImplicitNode> root = gm::ImplicitBox::create({-2.f, -2.f, -2.f}, {2.f, 2.f, 2.f});
    root = gm::ImplicitDifference::create(root, gm::ImplicitSphere::create({1.f, 0.f, 0.f}, 2.f));
    root = gm::ImplicitHull::create(root, 0.2f);

    m_imp_tree = gm::ImplicitTree::create(root);

    m_timer.start();
    m_mImplicit = m_imp_tree->polygonize(m_implicit_resolution, m_imp_box);
    m_timer.stop();
    
    m_ipolytms = m_timer.ms();
    m_ipolytus = m_timer.us();

    if (m_implicit_demo)
    {
        center_camera(*m_mImplicit_box);
    }

    m_mImplicit_box = m_imp_box.get_box(m_implicit_resolution, m_slide_x, m_slide_y, m_slide_z);

    // auto start= std::chrono::high_resolution_clock::now();
    // m_teapot.load_pacthes(std::string(DATA_DIR) + "/teapot");
    // m_mTeapot= m_teapot.polygonize(4);
    // auto stop= std::chrono::high_resolution_clock::now();

    // m_patch_time_polygonize= float(std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());

    // Point pmin, pmax;
    // m_mTeapot.bounds(pmin, pmax);
    // m_camera.lookat(pmin, pmax);

    return 0;
}

int Viewer::init_imgui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Dear ImGui style
    // ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    m_expr_patch = exprtk_wrapper_init();
    add_double_variable(m_expr_patch, "u");
    add_double_variable(m_expr_patch, "v");
    set_expression_count(m_expr_patch, 3);

    m_expr_spline = exprtk_wrapper_init();
    add_double_variable(m_expr_spline, "t");
    add_double_variable(m_expr_spline, "a");
    set_expression_count(m_expr_spline, 4);

    return 0;
}

int Viewer::render()
{
    if (render_ui() < 0)
    {
        utils::error("Error with the UI rendering!");
        return -1;
    }

    m_framebuffer.bind();
    glClearColor(0.678f, 0.686f, 0.878f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
    glEnable(GL_DEPTH_TEST);

    if (render_any() < 0)
    {
        utils::error("Error with the geometry rendering!");
        return -1;
    }

    m_framebuffer.unbind();

    return 1;
}

int Viewer::quit_any()
{
    m_grid->release();
    m_mPatch->release();
    m_mSpline->release();
    // m_mTeapot->release();
    m_mImplicit->release();

    return 0;
}

int Viewer::quit_imgui()
{
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    delete_exprtk(m_expr_spline);
    delete_exprtk(m_expr_patch);

    return 0;
}

int Viewer::render_any()
{
    Transform model = Identity();
    Transform view = m_camera.view();
    Transform projection = m_camera.projection();

    Transform mvp = projection * view * model;

    DrawParam param;
    param.model(model).view(view).projection(projection);

    if (m_spline_demo)
    {
        if (m_show_faces_spline)
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(1.0, 1.0);
            glDepthFunc(GL_LESS);
            param.draw(*m_mSpline);
            glDisable(GL_POLYGON_OFFSET_FILL);
        }

        if (m_show_edges_spline)
        {
            glUseProgram(m_program_edges);

            glLineWidth(m_size_edge);
            program_uniform(m_program_edges, "uMvpMatrix", mvp);
            GLint location = glGetUniformLocation(m_program_edges, "uEdgeColor");
            glUniform4fv(location, 1, &m_color_edge[0]);

            m_mSpline->draw(m_program_edges, true, false, false, false, false);
        }

        if (m_show_points_spline)
        {
            glUseProgram(m_program_points);

            program_uniform(m_program_points, "uMvpMatrix", mvp);
            program_uniform(m_program_points, "uPointSize", m_size_point);
            GLint location = glGetUniformLocation(m_program_points, "uPointColor");
            glUniform4fv(location, 1, &m_color_point[0]);

            glDrawArrays(GL_POINTS, 0, m_mSpline->vertex_count());
        } 

        if (m_show_spline_curve)
        {
            param.draw(*m_line);
        }
    }
    else if (m_patch_demo)
    {
        if (m_show_faces_patch)
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(1.0, 1.0);
            glDepthFunc(GL_LESS);
            param.draw(*m_mPatch);
            glDisable(GL_POLYGON_OFFSET_FILL);
        }

        if (m_show_edges_patch)
        {
            glUseProgram(m_program_edges);

            glLineWidth(m_size_edge);
            program_uniform(m_program_edges, "uMvpMatrix", mvp);
            GLint location = glGetUniformLocation(m_program_edges, "uEdgeColor");
            glUniform4fv(location, 1, &m_color_edge[0]);

            m_mPatch->draw(m_program_edges, true, false, false, false, false);
        }

        if (m_show_points_patch)
        {
            glUseProgram(m_program_points);

            program_uniform(m_program_points, "uMvpMatrix", mvp);
            program_uniform(m_program_points, "uPointSize", m_size_point);
            GLint location = glGetUniformLocation(m_program_points, "uPointColor");
            glUniform4fv(location, 1, &m_color_point[0]);

            glDrawArrays(GL_POINTS, 0, m_mPatch->vertex_count());
        }    

        if (m_show_patch_grid)
        {
            param.draw(*m_grid);
        }
        // param.draw(*m_mTeapot);
    }
    else if (m_implicit_demo)
    {
        if (m_show_faces_implicit)
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(1.0, 1.0);
            glDepthFunc(GL_LESS);
            param.draw(*m_mImplicit);
            glDisable(GL_POLYGON_OFFSET_FILL);
        }

        if (m_show_edges_implicit)
        {
            glUseProgram(m_program_edges);

            glLineWidth(m_size_edge);
            program_uniform(m_program_edges, "uMvpMatrix", mvp);
            GLint location = glGetUniformLocation(m_program_edges, "uEdgeColor");
            glUniform4fv(location, 1, &m_color_edge[0]);

            m_mImplicit->draw(m_program_edges, true, false, false, false, false);
        }

        if (m_show_points_implicit)
        {
            glUseProgram(m_program_points);

            program_uniform(m_program_points, "uMvpMatrix", mvp);
            program_uniform(m_program_points, "uPointSize", m_size_point);
            GLint location = glGetUniformLocation(m_program_points, "uPointColor");
            glUniform4fv(location, 1, &m_color_point[0]);

            glDrawArrays(GL_POINTS, 0, m_mImplicit->vertex_count());
        }    
        
        if (m_show_implicit_box)
        {
            param.draw(*m_mImplicit_box);
        }
    }


    return 0;
}

int Viewer::render_ui()
{
    ImGui::DockSpaceOverViewport();

    if (render_menu_bar() < 0)
    {
        utils::error("Error with the menu bar rendering!");
        return -1;
    }

    ImGui::Begin("Scene");

    if (ImGui::IsWindowHovered())
    {
        ImGuiIO &io = ImGui::GetIO();
        (void)io;

        io.WantCaptureMouse = false;
    }

    // we access the ImGui window size
    const float window_width = ImGui::GetContentRegionAvail().x;
    const float window_height = ImGui::GetContentRegionAvail().y;

    // we rescale the framebuffer to the actual window size here and reset the glViewport
    m_framebuffer.rescale(window_width, window_height);
    glViewport(0, 0, window_width, window_height);
    m_camera.projection(window_width, window_height, 45);

    // we get the screen position of the window
    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::GetWindowDrawList()->AddImage(
        (void *)m_framebuffer.texture_id(),
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + window_width, pos.y + window_height),
        ImVec2(0, 1),
        ImVec2(1, 0));

    ImGui::End();

    if (m_show_ui)
    {
        ImGui::Begin("Parameters");

        render_demo_buttons();

        ImGui::SliderFloat("Point size", &m_size_point, 1.f, 50.f, "%.2f");
        ImGui::SliderFloat("Edge size", &m_size_edge, 1.f, 25.f, "%.2f");
        if (m_spline_demo) render_spline_params();
        else if (m_patch_demo) render_patch_params();
        else if (m_implicit_demo) render_implicit_params();
        ImGui::ColorPicker3("Point color", &m_color_point[0]);
        ImGui::ColorPicker3("Edge color", &m_color_edge[0]);
        ImGui::End();

        ImGui::Begin("Statistiques");
        ImGui::SeparatorText("Performances");
        auto [cpums, cpuus] = cpu_time();
        auto [gpums, gpuus] = gpu_time();
        ImGui::Text("fps : %.2f ", (1000.f / delta_time()));
        ImGui::Text("cpu : %i ms %i us ", cpums, cpuus);
        ImGui::Text("gpu : %i ms %i us", gpums, gpuus);
        ImGui::Text("total : %.2f ms", delta_time());
        if (m_spline_demo) render_spline_stats();
        else if (m_patch_demo) render_patch_stats();
        else if (m_implicit_demo) render_implicit_stats();
        ImGui::End();
    }

    if (m_show_style_editor)
    {
        ImGui::Begin("Dear ImGui Style Editor", &m_show_style_editor);
        ImGui::ShowStyleEditor();
        ImGui::End();
    }

    ImGui::Render();

    return 0;
}

int Viewer::render_demo_buttons()
{
    ImVec2 sz = ImVec2(-FLT_MIN, 45.0f);

    //! Spline demo widget
    ImGui::BeginDisabled(m_spline_demo);
    if (ImGui::Button("Spline Demo", sz) && !m_spline_demo)
    {
        m_patch_demo = false;
        m_spline_demo = true;
        m_implicit_demo = false;

        center_camera(*m_mSpline);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
    {
        ImGui::SetTooltip(!m_spline_demo ? "Activate spline demo." : "Spline demo is active.");
    }
    ImGui::EndDisabled();

    //! Patch demo widget
    ImGui::BeginDisabled(m_patch_demo);
    if (ImGui::Button("Patch Demo", sz) && !m_patch_demo)
    {
        m_patch_demo = true;
        m_spline_demo = false;
        m_implicit_demo = false;

        center_camera(*m_mPatch);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
    {
        ImGui::SetTooltip(!m_patch_demo ? "Activate patch demo." : "Patch demo is active.");
    }
    ImGui::EndDisabled();

    //! Implicit demo widget
    ImGui::BeginDisabled(m_implicit_demo);
    if (ImGui::Button("Implicit Demo", sz) && !m_implicit_demo)
    {
        m_patch_demo = false;
        m_spline_demo = false;
        m_implicit_demo = true;

        center_camera(*m_mImplicit_box);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
    {
        ImGui::SetTooltip(!m_implicit_demo ? "Activate patch demo." : "Patch demo is active.");
    }
    ImGui::EndDisabled();

    return 0;
}

int Viewer::render_patch_stats()
{
    //! Statistiques
    ImGui::SeparatorText("Patch Geometry");
    // ImGui::Text("#Triangle : %i ", m_mTeapot->triangle_count());
    ImGui::Text("#Triangle : %i ", m_mPatch->triangle_count());
    // ImGui::Text("#vertex : %i ", m_mTeapot->vertex_count());
    ImGui::Text("#vertex : %i ", m_mPatch->vertex_count());
    ImGui::Text("#Control points : %i ", m_patch->point_count());
    ImGui::Text("Poligonize Time : %i ms %i us", m_ppolytms, m_ppolytus);

    return 0;
}

int Viewer::render_patch_params()
{
    ImVec2 sz = ImVec2(-FLT_MIN, 45.0f);

    ImGui::SeparatorText("Bézier Patch");
    ImGui::SliderInt("Resolution", &m_patch_resolution, 4, 1000);
    ImGui::SliderInt("#Control points", &m_nb_control_points_patch, 4, 31);
    ImGui::Text("You can use the two variables 'u' and 'v' in each expression below :");
    ImGui::InputText("X", surface_function_input_x, IM_ARRAYSIZE(surface_function_input_x));
    ImGui::InputText("Y", surface_function_input_y, IM_ARRAYSIZE(surface_function_input_y));
    ImGui::InputText("Z", surface_function_input_z, IM_ARRAYSIZE(surface_function_input_z));
    ImGui::Checkbox("Faces", &m_show_faces_patch);
    ImGui::SameLine();
    ImGui::Checkbox("Edges", &m_show_edges_patch);
    ImGui::SameLine();
    ImGui::Checkbox("Points", &m_show_points_patch);
    ImGui::SameLine();
    ImGui::Checkbox("Grid", &m_show_patch_grid);
    ImGui::NewLine();
    if (ImGui::Button("Render", sz))
    {

        set_expression_string(m_expr_patch, surface_function_input_x, 0);
        set_expression_string(m_expr_patch, surface_function_input_y, 1);
        set_expression_string(m_expr_patch, surface_function_input_z, 2);

        if (!compile_expression(m_expr_patch))
        {
            std::cout << "[Patch] Error in function parsing." << std::endl;
            exit(1);
        }

        std::vector<std::vector<Point>> surface;
        surface = gm::surface_points(m_nb_control_points_patch, [&](double u_val, double v_val)
                                     {
                    set_double_variable_value(m_expr_patch, "u", u_val);
                    set_double_variable_value(m_expr_patch, "v", v_val);
                    double x= get_evaluated_value(m_expr_patch, 0); 
                    double y= get_evaluated_value(m_expr_patch, 1); 
                    double z= get_evaluated_value(m_expr_patch, 2); 
                    return Point(x, y, z); });

        m_grid->clear();
        for (int h = 1; h < m_nb_control_points_patch; ++h)
        {
            for (int w = 1; w < m_nb_control_points_patch; ++w)
            {
                m_grid->vertex(surface[h][w - 1]);
                m_grid->vertex(surface[h][w]);

                m_grid->vertex(surface[h - 1][w]);
                m_grid->vertex(surface[h][w]);
            }
        }

        m_patch = gm::Bezier::create(surface);

        m_timer.start();
        m_mPatch = m_patch->polygonize(m_patch_resolution);
        // m_mTeapot = m_teapot.polygonize(m_patch_resolution);
        m_timer.stop();

        m_ppolytms = m_timer.ms();
        m_ppolytus = m_timer.us();

        center_camera(*m_mPatch);
    }

    return 0;
}

int Viewer::render_spline_stats()
{
    ImGui::SeparatorText("Spline Geometry");
    ImGui::Text("#Triangle : %i ", m_mSpline->triangle_count());
    ImGui::Text("#vertex : %i ", m_mSpline->vertex_count());
    ImGui::Text("#Control points : %i ", m_spline->point_count());
    ImGui::Text("Poligonize Time : %i ms %i us", m_spolytms, m_spolytus);

    return 0;
}

int Viewer::render_spline_params()
{
    ImVec2 sz = ImVec2(-FLT_MIN, 45.0f);

    ImGui::SeparatorText("Bézier Spline");
    ImGui::SliderInt("Resolution", &m_spline_resolution, 3, 1000);
    ImGui::SliderInt("#Control points", &m_nb_control_points_spline, 2, 31);
    ImGui::Text("You can use a variable 't' in each expression below :");
    ImGui::InputText("X", curve_function_input_x, IM_ARRAYSIZE(curve_function_input_x));
    ImGui::InputText("Y", curve_function_input_y, IM_ARRAYSIZE(curve_function_input_y));
    ImGui::InputText("Z", curve_function_input_z, IM_ARRAYSIZE(curve_function_input_z));
    ImGui::Checkbox("Faces", &m_show_faces_spline);
    ImGui::SameLine();
    ImGui::Checkbox("Edges", &m_show_edges_spline);
    ImGui::SameLine();
    ImGui::Checkbox("Points", &m_show_points_spline);
    ImGui::SameLine();
    ImGui::Checkbox("Curve", &m_show_spline_curve);
    ImGui::Text("You can use the two variables 't' and 'a' (angle value) in the expression below :");
    ImGui::InputText("Radial Function", spline_radial_function_input, IM_ARRAYSIZE(spline_radial_function_input));
    if (ImGui::Button("Render", sz))
    {
        set_expression_string(m_expr_spline, curve_function_input_x, 0);
        set_expression_string(m_expr_spline, curve_function_input_y, 1);
        set_expression_string(m_expr_spline, curve_function_input_z, 2);
        set_expression_string(m_expr_spline, spline_radial_function_input, 3);

        if (!compile_expression(m_expr_spline))
        {
            std::cout << "[Spline] Error in function parsing." << std::endl;
            exit(1);
        }

        std::vector<Point> curve;
        curve = gm::curve_points(m_nb_control_points_spline, [&](double t_val)
                                 {
                    set_double_variable_value(m_expr_spline, "t", t_val);
                    double x= get_evaluated_value(m_expr_spline, 0); 
                    double y= get_evaluated_value(m_expr_spline, 1); 
                    double z= get_evaluated_value(m_expr_spline, 2); 
                    return Point(x, y, z); });

        m_line->clear();
        for (const auto &p : curve)
        {
            m_line->vertex(p);
        }

        m_spline = gm::Revolution::create(curve);
        m_spline->radial_fun([&](double t_val, double a_val)
                            {
                        set_double_variable_value(m_expr_spline, "t", t_val);
                        set_double_variable_value(m_expr_spline, "a", a_val);
                        return get_evaluated_value(m_expr_spline, 3); });

        m_timer.start();
        m_mSpline = m_spline->polygonize(m_spline_resolution);
        m_timer.stop();

        m_spolytms = m_timer.ms();
        m_spolytus = m_timer.us();

        center_camera(*m_mSpline);
    }

    return 0;
}

int Viewer::render_implicit_stats()
{
    ImGui::SeparatorText("Spline Geometry");
    ImGui::Text("#Triangle : %i ", m_mImplicit->triangle_count());
    ImGui::Text("#vertex : %i ", m_mImplicit->vertex_count());
    ImGui::Text("Poligonize Time : %i ms %i us", m_ipolytms, m_ipolytus);
    return 0;
}

int Viewer::render_implicit_params()
{
    ImVec2 sz = ImVec2(-FLT_MIN, 45.0f);

    ImGui::SeparatorText("Implicit Surface");
    if (ImGui::SliderInt("Resolution", &m_implicit_resolution, 3, 1000))
    {
        m_slide_x = 0;
        m_slide_y = 0;
        m_slide_z = 0;
    }
    if (ImGui::SliderInt("Box slide x", &m_slide_x, 0, m_implicit_resolution))
    {
        m_mImplicit_box = m_imp_box.get_box(m_implicit_resolution, m_slide_x, m_slide_y, m_slide_z);
    }
    if (ImGui::SliderInt("Box slide y", &m_slide_y, 0, m_implicit_resolution))
    {
        m_mImplicit_box = m_imp_box.get_box(m_implicit_resolution, m_slide_x, m_slide_y, m_slide_z);
    }
    if (ImGui::SliderInt("Box slide z", &m_slide_z, 0, m_implicit_resolution))
    {
        m_mImplicit_box = m_imp_box.get_box(m_implicit_resolution, m_slide_x, m_slide_y, m_slide_z);
    }
    ImGui::SliderFloat3("Pmin box", pmin, -10.f, 10.f, "%.2f");
    ImGui::SliderFloat3("Pmax box", pmax, -10.f, 10.f, "%.2f");
    ImGui::Checkbox("Faces", &m_show_faces_implicit);
    ImGui::SameLine();
    ImGui::Checkbox("Edges", &m_show_edges_implicit);
    ImGui::SameLine();
    ImGui::Checkbox("Points", &m_show_points_implicit);
    ImGui::SameLine();
    ImGui::Checkbox("implicit box", &m_show_implicit_box);
    if (ImGui::Button("Render", sz))
    {
        m_imp_box.a({pmin[0], pmin[1], pmin[2]});
        m_imp_box.b({pmax[0], pmax[1], pmax[2]});

        m_mImplicit_box = m_imp_box.get_box(m_implicit_resolution, m_slide_x, m_slide_y, m_slide_z);

        m_timer.start();
        m_mImplicit = m_imp_tree->polygonize(m_implicit_resolution, m_imp_box);
        m_timer.stop();

        m_ipolytms = m_timer.ms();
        m_ipolytus = m_timer.us();

        center_camera(*m_mImplicit_box);
    }

    return 0;
}

int Viewer::render_menu_bar()
{
    ImGui::DockSpace(ImGui::GetID("DockSpace"));

    if (ImGui::BeginMainMenuBar())
    {
        // ImGui::MenuItem("Style Editor", NULL, &m_show_style_editor);
        ImGui::MenuItem("Exit", NULL, &m_exit);
        ImGui::MenuItem(m_show_ui ? "Hide UI" : "Show UI", NULL, &m_show_ui);
        if (ImGui::MenuItem(m_dark_theme ? "Light Theme" : "Dark Theme", NULL, &m_dark_theme))
        {
            if (m_dark_theme)
            {
                ImGui::StyleColorsDark();
            }
            else
            {
                ImGui::StyleColorsLight();
            }
        }

        ImGui::EndMainMenuBar();
    }

    // ImGui::ShowDemoWindow();

    return 0;
}
