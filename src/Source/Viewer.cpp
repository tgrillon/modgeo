#include "Viewer.h"

#include "Utils.h"

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
    if (init_demo_spline() < 0)
    {
        utils::error("in [init_demo_spline]");
    }

    //! Bezier patch intialization
    if (init_demo_patch() < 0)
    {
        utils::error("in [init_demo_patch]");
    }

    //! SDF surface initialization
    if (init_demo_sdf() < 0)
    {
        utils::error("in [init_demo_sdf]");
    }

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
    m_mSDF->release();

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

int Viewer::init_demo_patch()
{
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

    return 0;
}

int Viewer::init_demo_spline()
{
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

    return 0;
}

int Viewer::init_demo_sdf()
{
    m_sdf_root = gm::SDFTorus::create(0.5, 0.2);
    m_sdf_tree = gm::SDFTree::create(m_sdf_root);

    m_timer.start();
    m_mSDF = m_sdf_tree->polygonize(m_sdf_resolution, m_sdf_box);
    m_timer.stop();

    m_ipolytms = m_timer.ms();
    m_ipolytus = m_timer.us();

    if (m_sdf_demo)
    {
        center_camera(*m_mSDF_box);
    }

    m_mSDF_box = m_sdf_box.get_box(m_sdf_resolution, m_slide_x, m_slide_y, m_slide_z);

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

    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    if (m_spline_demo)
    {
        if (!io.WantCaptureKeyboard && !io.WantCaptureMouse)
        {
            handle_event_spline();
        }

        if (m_mSpline->has_position())
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
        }

        if (m_show_spline_curve)
        {
            param.draw(*m_line);
        }
    }
    else if (m_patch_demo)
    {
        if (!io.WantCaptureKeyboard && !io.WantCaptureMouse)
        {
            handle_event_patch();
        }

        if (m_mPatch->has_position())
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
        }

        if (m_show_patch_grid)
        {
            param.draw(*m_grid);
        }
        // param.draw(*m_mTeapot);
    }
    else if (m_sdf_demo)
    {
        if (!io.WantCaptureKeyboard && !io.WantCaptureMouse)
        {
            handle_event_sdf();
        }

        if (m_mSDF->has_position())
        {
            if (m_show_faces_implicit)
            {
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(1.0, 1.0);
                glDepthFunc(GL_LESS);
                param.draw(*m_mSDF);
                glDisable(GL_POLYGON_OFFSET_FILL);
            }

            if (m_show_edges_implicit)
            {
                glUseProgram(m_program_edges);

                glLineWidth(m_size_edge);
                program_uniform(m_program_edges, "uMvpMatrix", mvp);
                GLint location = glGetUniformLocation(m_program_edges, "uEdgeColor");
                glUniform4fv(location, 1, &m_color_edge[0]);

                m_mSDF->draw(m_program_edges, true, false, false, false, false);
            }

            if (m_show_points_implicit)
            {
                glUseProgram(m_program_points);

                program_uniform(m_program_points, "uMvpMatrix", mvp);
                program_uniform(m_program_points, "uPointSize", m_size_point);
                GLint location = glGetUniformLocation(m_program_points, "uPointColor");
                glUniform4fv(location, 1, &m_color_point[0]);

                glDrawArrays(GL_POINTS, 0, m_mSDF->vertex_count());
            }
        }

        if (m_show_sdf_box)
        {
            param.draw(*m_mSDF_box);
        }
    }

    return 0;
}

int Viewer::handle_event_spline()
{

    if (key_state(SDLK_f))
    {
        clear_key_state(SDLK_f);
        m_show_faces_spline = !m_show_faces_spline;
    }

    if (key_state(SDLK_e))
    {
        clear_key_state(SDLK_e);
        m_show_edges_spline = !m_show_edges_spline;
    }

    if (key_state(SDLK_v))
    {
        clear_key_state(SDLK_v);
        m_show_points_spline = !m_show_points_spline;
    }

    return 0;
}

int Viewer::handle_event_patch()
{
    if (key_state(SDLK_f))
    {
        clear_key_state(SDLK_f);
        m_show_faces_patch = !m_show_faces_patch;
    }

    if (key_state(SDLK_e))
    {
        clear_key_state(SDLK_e);
        m_show_edges_patch = !m_show_edges_patch;
    }

    if (key_state(SDLK_v))
    {
        clear_key_state(SDLK_v);
        m_show_points_patch = !m_show_points_patch;
    }

    return 0;
}

int Viewer::handle_event_sdf()
{
    if (key_state(SDLK_f))
    {
        clear_key_state(SDLK_f);
        m_show_faces_implicit = !m_show_faces_implicit;
    }

    if (key_state(SDLK_e))
    {
        clear_key_state(SDLK_e);
        m_show_edges_implicit = !m_show_edges_implicit;
    }

    if (key_state(SDLK_v))
    {
        clear_key_state(SDLK_v);
        m_show_points_implicit = !m_show_points_implicit;
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

    ImGui::Begin("Viewport");

    if (ImGui::IsWindowHovered())
    {
        ImGuiIO &io = ImGui::GetIO();
        (void)io;

        io.WantCaptureMouse = false;
        io.WantCaptureKeyboard = false;
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
        ImGui::Begin("Control Panel");

        render_demo_buttons();
        if (ImGui::CollapsingHeader("Global"))
        {
            ImGui::SliderFloat("Point size", &m_size_point, 1.f, 50.f, "%.2f");
            ImGui::SliderFloat("Edge size", &m_size_edge, 1.f, 25.f, "%.2f");
            if (ImGui::CollapsingHeader("Colors"))
            {
                ImGui::ColorPicker3("Point color", &m_color_point[0]);
                ImGui::ColorPicker3("Edge color", &m_color_edge[0]);
            }
        }
        if (ImGui::CollapsingHeader("Params"))
        {
            if (m_spline_demo)
                render_params_spline();
            else if (m_patch_demo)
                render_params_patch();
            else if (m_sdf_demo)
                render_params_sdf();
        }

        ImGui::InputTextWithHint("Filename", "ex : my_mesh", &m_filename);
        if (ImGui::Button("Save mesh"))
        {

            if (m_filename.empty())
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(1000, 9999);

                if (m_spline_demo)
                    m_filename = "spline_";
                else if (m_patch_demo)
                    m_filename = "patch_";
                else if (m_sdf_demo)
                    m_filename = "sdf_";

                int id = dis(gen);
                m_filename = m_filename + std::to_string(id);
            }

            std::string fullpath = std::string(OBJ_DIR) + "/" + m_filename + ".obj";

            if (m_spline_demo)
                save_spline(fullpath);
            else if (m_patch_demo)
                save_patch(fullpath);
            else if (m_sdf_demo)
                save_sdf(fullpath);

            m_filename = "";
        }
        ImGui::End();

        ImGui::Begin("Statistiques");
        if (ImGui::CollapsingHeader("Performances"))
        {
            auto [cpums, cpuus] = cpu_time();
            auto [gpums, gpuus] = gpu_time();
            ImGui::Text("fps : %.2f ", (1000.f / delta_time()));
            ImGui::Text("cpu : %i ms %i us ", cpums, cpuus);
            ImGui::Text("gpu : %i ms %i us", gpums, gpuus);
            ImGui::Text("frame rate : %.2f ms", delta_time());
        }
        if (ImGui::CollapsingHeader("Geometry"))
        {
            if (m_spline_demo)
                render_stats_spline();
            else if (m_patch_demo)
                render_stats_patch();
            else if (m_sdf_demo)
                render_stats_sdf();
        }
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

    if (ImGui::CollapsingHeader("Menu"))
    {
        //! Spline demo widget
        ImGui::BeginDisabled(m_spline_demo);
        if (ImGui::Button("Spline Demo", sz) && !m_spline_demo)
        {
            m_patch_demo = false;
            m_spline_demo = true;
            m_sdf_demo = false;

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
            m_sdf_demo = false;

            center_camera(*m_mPatch);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip(!m_patch_demo ? "Activate patch demo." : "Patch demo is active.");
        }
        ImGui::EndDisabled();

        //! SDF demo widget
        ImGui::BeginDisabled(m_sdf_demo);
        if (ImGui::Button("SDF Demo", sz) && !m_sdf_demo)
        {
            m_patch_demo = false;
            m_spline_demo = false;
            m_sdf_demo = true;

            center_camera(*m_mSDF_box);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            ImGui::SetTooltip(!m_sdf_demo ? "Activate patch demo." : "Patch demo is active.");
        }
        ImGui::EndDisabled();
    }

    return 0;
}

int Viewer::render_stats_patch()
{
    //! Statistiques
    ImGui::SeparatorText("GEOMETRY");
    // ImGui::Text("#Triangle : %i ", m_mTeapot->triangle_count());
    ImGui::Text("#Triangle : %i ", m_mPatch->triangle_count());
    // ImGui::Text("#vertex : %i ", m_mTeapot->vertex_count());
    ImGui::Text("#vertex : %i ", m_mPatch->vertex_count());
    ImGui::Text("#Control points : %i ", m_patch->point_count());
    ImGui::Text("Poligonize Time : %i ms %i us", m_ppolytms, m_ppolytus);

    return 0;
}

int Viewer::render_params_patch()
{
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
    if (ImGui::Button("Render"))
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

int Viewer::render_stats_spline()
{
    ImGui::SeparatorText("GEOMETRY");
    ImGui::Text("#Triangle : %i ", m_mSpline->triangle_count());
    ImGui::Text("#vertex : %i ", m_mSpline->vertex_count());
    ImGui::Text("#Control points : %i ", m_spline->point_count());
    ImGui::Text("Poligonize Time : %i ms %i us", m_spolytms, m_spolytus);

    return 0;
}

int Viewer::render_params_spline()
{
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
    if (ImGui::Button("Render"))
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

int Viewer::render_stats_sdf()
{
    ImGui::SeparatorText("GEOMETRY");
    ImGui::Text("#Triangle : %i ", m_mSDF->triangle_count());
    ImGui::Text("#vertex : %i ", m_mSDF->vertex_count());
    ImGui::Text("Poligonize Time : %i ms %i us", m_ipolytms, m_ipolytus);
    ImGui::Text("Value call count : %i", m_sdf_tree->value_call_count());
    return 0;
}

int Viewer::render_params_sdf()
{
    ImVec2 sz = ImVec2(-FLT_MIN, 45.0f);

    ImGui::SeparatorText("SDF TREE");
    build_sdf_tree();

    if (ImGui::CollapsingHeader("SDF Space"))
    {
        if (ImGui::SliderInt("Box slide x", &m_slide_x, 0, m_sdf_resolution))
        {
            m_mSDF_box = m_sdf_box.get_box(m_sdf_resolution, m_slide_x, m_slide_y, m_slide_z);
        }
        if (ImGui::SliderInt("Box slide y", &m_slide_y, 0, m_sdf_resolution))
        {
            m_mSDF_box = m_sdf_box.get_box(m_sdf_resolution, m_slide_x, m_slide_y, m_slide_z);
        }
        if (ImGui::SliderInt("Box slide z", &m_slide_z, 0, m_sdf_resolution))
        {
            m_mSDF_box = m_sdf_box.get_box(m_sdf_resolution, m_slide_x, m_slide_y, m_slide_z);
        }
        ImGui::SliderFloat3("Pmin box", pmin, -10.f, 10.f, "%.2f");
        ImGui::SliderFloat3("Pmax box", pmax, -10.f, 10.f, "%.2f");
    }
    ImGui::Checkbox("Faces", &m_show_faces_implicit);
    ImGui::SameLine();
    ImGui::Checkbox("Edges", &m_show_edges_implicit);
    ImGui::SameLine();
    ImGui::Checkbox("Points", &m_show_points_implicit);
    ImGui::SameLine();
    ImGui::Checkbox("implicit space", &m_show_sdf_box);

    if (ImGui::SliderInt("Resolution", &m_sdf_resolution, 3, 1000))
    {
        m_slide_x = 0;
        m_slide_y = 0;
        m_slide_z = 0;
    }

    render_sdf_buttons();

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

void Viewer::save_spline(const std::string &fullpath) const
{
    write_mesh(*m_mSpline, fullpath.c_str());
}

void Viewer::save_patch(const std::string &fullpath) const
{
    write_mesh(*m_mPatch, fullpath.c_str());
}

void Viewer::save_sdf(const std::string &fullpath) const
{
    write_mesh(*m_mSDF, fullpath.c_str());
}

void Viewer::set_sdf_primitive()
{
    ImGui::Text("Selected primitive : %s", (m_sdf_node ? gm::type_str(m_sdf_node->type()) : "NULL"));
    ImGui::Text("Selected Operator : %s", (m_sdf_root ? gm::type_str(m_sdf_root->type()) : "NULL"));
    if (ImGui::CollapsingHeader("Select Primitive"))
    {
        if (ImGui::TreeNode("Sphere"))
        {
            static float radius = 1.0f;
            ImGui::InputFloat("Radius", &radius);

            if (ImGui::Button("Add Sphere"))
            {
                m_sdf_node = gm::SDFSphere::create({0, 0, 0}, radius);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Box"))
        {
            static float pmin[3] = {-1.0, -1.0, -1.0}, pmax[3] = {1.0, 1.0, 1.0};
            ImGui::InputFloat3("Min Point", &pmin[0]);
            ImGui::InputFloat3("Max Point", &pmax[0]);

            if (ImGui::Button("Add Box"))
            {
                m_sdf_node = gm::SDFBox::create({pmin[0], pmin[1], pmin[2]}, {pmax[0], pmax[1], pmax[2]});
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Torus"))
        {
            static float R, r;
            ImGui::InputFloat("R", &R);
            ImGui::InputFloat("r", &r);

            if (ImGui::Button("Add Torus"))
            {
                m_sdf_node = gm::SDFTorus::create(R, r);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Plane"))
        {
            static float n[3] = {0.0, 1.0, 0.0};
            static float h = 1.0;
            ImGui::InputFloat3("normal", &n[0]);
            ImGui::InputFloat("h", &h);
            if (ImGui::Button("Add Plane"))
            {
                m_sdf_node = gm::SDFPlane::create({n[0], n[1], n[2]}, h);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Capsule"))
        {
            static float r = 0.5;
            static float h = 1.0;
            ImGui::InputFloat("r", &h);
            ImGui::InputFloat("h", &r);
            if (ImGui::Button("Add Capsule"))
            {
                m_sdf_node = gm::SDFCapsule::create(r, h);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Cylinder"))
        {
            static float r = 0.5;
            static float h = 1.0;
            ImGui::InputFloat("r", &h);
            ImGui::InputFloat("h", &r);
            if (ImGui::Button("Add Cylinder"))
            {
                m_sdf_node = gm::SDFCylinder::create(r, h);
            }
            ImGui::TreePop();
        }
    }
}

void Viewer::set_sdf_operator()
{
    if (ImGui::CollapsingHeader("Select Operator"))
    {
        ImGui::BeginDisabled(m_sdf_root == nullptr || m_sdf_node == nullptr);
        if (ImGui::CollapsingHeader("Binary Operator"))
        {
            if (ImGui::TreeNode("Union"))
            {
                if (ImGui::Button("Add Union"))
                {
                    m_sdf_root = gm::SDFUnion::create(m_sdf_root, m_sdf_node);
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Intersection"))
            {
                if (ImGui::Button("Add Intersection"))
                {
                    m_sdf_root = gm::SDFIntersection::create(m_sdf_root, m_sdf_node);
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Substraction"))
            {
                if (ImGui::Button("Add Substraction"))
                {
                    m_sdf_root = gm::SDFSubstraction::create(m_sdf_root, m_sdf_node);
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Smooth Union"))
            {
                ImGui::SliderFloat("K (smooth constante)", &m_smooth_k, 0.f, 1.f);
                if (ImGui::Button("Add Smooth Union"))
                {
                    m_sdf_root = gm::SDFSmoothUnion::create(m_sdf_root, m_sdf_node, m_smooth_k);
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Smooth Intersection"))
            {
                ImGui::SliderFloat("K (smooth constante)", &m_smooth_k, 0.f, 1.f);
                if (ImGui::Button("Add Smooth Intersection"))
                {
                    m_sdf_root = gm::SDFSmoothIntersection::create(m_sdf_root, m_sdf_node, m_smooth_k);
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Smooth Substraction"))
            {
                ImGui::SliderFloat("K (smooth constante)", &m_smooth_k, 0.f, 1.f);
                if (ImGui::Button("Add Smooth Substraction"))
                {
                    m_sdf_root = gm::SDFSmoothSubstraction::create(m_sdf_root, m_sdf_node, m_smooth_k);
                }
                ImGui::TreePop();
            }
        }
        ImGui::EndDisabled();

        auto &node = m_sdf_root ? m_sdf_root : m_sdf_node;
        if (ImGui::CollapsingHeader("Unary Operator"))
        {
            if (ImGui::TreeNode("Hull"))
            {
                static float thickness = 0.1f;
                ImGui::InputFloat("Thickness", &thickness);
                if (ImGui::Button("Add Hull"))
                {
                    node = gm::SDFHull::create(node, thickness);
                }
                ImGui::TreePop();
            }

            if (ImGui::CollapsingHeader("Transform"))
            {
                if (ImGui::TreeNode("Translation"))
                {
                    static Vector t = {0., 0., 0.};
                    ImGui::InputFloat3("Translation vector", &t.x);
                    if (ImGui::Button("Add Translation"))
                    {
                        node = gm::SDFTranslation::create(node, t);
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Rotation"))
                {
                    static Vector axis = {0., 0., 0.};
                    static float angle = 0.;
                    ImGui::InputFloat("Angle (in degrees)", &angle);
                    ImGui::InputFloat3("Axis", &axis.x);
                    if (ImGui::Button("Add Rotation"))
                    {
                        node = gm::SDFRotation::create(node, axis, angle);
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("RotationX"))
                {
                    static float angle = 0.;
                    ImGui::InputFloat("Angle (in degrees)", &angle);
                    if (ImGui::Button("Add RotationX"))
                    {
                        node = gm::SDFRotationX::create(node, angle);
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("RotationY"))
                {
                    static float angle = 0.;
                    ImGui::InputFloat("Angle (in degrees)", &angle);
                    if (ImGui::Button("Add RotationY"))
                    {
                        node = gm::SDFRotationY::create(node, angle);
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("RotationZ"))
                {
                    static float angle = 0.;
                    ImGui::InputFloat("Angle (in degrees)", &angle);
                    if (ImGui::Button("Add RotationZ"))
                    {
                        node = gm::SDFRotationZ::create(node, angle);
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Scale"))
                {
                    static float scale = 0.;
                    ImGui::InputFloat("Scale", &scale);
                    if (ImGui::Button("Add Scale"))
                    {
                        node = gm::SDFScale::create(node, scale);
                    }
                    ImGui::TreePop();
                }
            }
        }
    }
}

void Viewer::build_sdf_tree()
{
    if (ImGui::CollapsingHeader("Build Tree"))
    {
        set_sdf_primitive();
        set_sdf_operator();
    }

    if (ImGui::CollapsingHeader("Modify Tree"))
    {
        render_node_ui(m_sdf_tree->root());
    }
}

void Viewer::render_node_ui(Ref<gm::SDFNode> &node)
{
    if (!node)
        return;

    if (ImGui::TreeNode(type_str(node->type())))
    {
        if (auto sphere = dynamic_cast<gm::SDFSphere *>(node.get()))
        {
            ImGui::InputFloat3("Center", &sphere->center());
            ImGui::InputFloat("Radius", &sphere->radius());
        }
        else if (auto box = dynamic_cast<gm::SDFBox *>(node.get()))
        {
            ImGui::InputFloat3("Min Point", &box->pmin());
            ImGui::InputFloat3("Max Point", &box->pmax());
        }
        else if (auto torus = dynamic_cast<gm::SDFTorus *>(node.get()))
        {
            ImGui::InputFloat("R", &torus->R());
            ImGui::InputFloat("r", &torus->r());
        }
        else if (auto plane = dynamic_cast<gm::SDFPlane *>(node.get()))
        {
            ImGui::InputFloat3("Normal", &plane->normal());
            ImGui::InputFloat("Height", &plane->height());
        }
        else if (auto capsule = dynamic_cast<gm::SDFCapsule *>(node.get()))
        {
            ImGui::InputFloat("Radius", &capsule->radius());
            ImGui::InputFloat("Height", &capsule->height());
        }
        else if (auto cylinder = dynamic_cast<gm::SDFCylinder *>(node.get()))
        {
            ImGui::InputFloat("Radius", &cylinder->radius());
            ImGui::InputFloat("Height", &cylinder->height());
        }
        else if (auto translation = dynamic_cast<gm::SDFTranslation *>(node.get()))
        {
            ImGui::InputFloat3("Translation", &translation->translation());
        }
        else if (auto rotation_x = dynamic_cast<gm::SDFRotationX *>(node.get()))
        {
            ImGui::InputFloat("Angle (in degrees)", &rotation_x->angle());
        }
        else if (auto rotation_y = dynamic_cast<gm::SDFRotationY *>(node.get()))
        {
            ImGui::InputFloat("Angle (in degrees)", &rotation_y->angle());
        }
        else if (auto rotation_z = dynamic_cast<gm::SDFRotationZ *>(node.get()))
        {
            ImGui::InputFloat("Angle (in degrees)", &rotation_z->angle());
        }
        else if (auto rotation = dynamic_cast<gm::SDFRotation *>(node.get()))
        {
            ImGui::InputFloat("Angle (in degrees)", &rotation->angle());
            ImGui::InputFloat3("Axis", &rotation->axis());
        }
        else if (auto scale = dynamic_cast<gm::SDFScale *>(node.get()))
        {
            ImGui::InputFloat("Scale", &scale->scale());
        }
        else if (auto smoothOP = dynamic_cast<gm::SDFSmoothBinaryOperator *>(node.get()))
        {
            ImGui::SliderFloat("K", &smoothOP->k(), 0.f, 1.f);
        }
        else if (auto hull = dynamic_cast<gm::SDFHull *>(node.get()))
        {
            ImGui::InputFloat("Thickness", &hull->thickness());
        }

        auto [left, right] = node->children();
        render_node_ui(left);
        render_node_ui(right);
        ImGui::TreePop();
    }
}

void Viewer::render_sdf_buttons()
{
    if (ImGui::Button("Render Tree"))
    {
        m_sdf_tree->reset_value_call_count();
        if (m_sdf_root == nullptr && m_sdf_node)
            m_sdf_root = m_sdf_node;

        if (m_sdf_root)
        {
            m_sdf_box.a({pmin[0], pmin[1], pmin[2]});
            m_sdf_box.b({pmax[0], pmax[1], pmax[2]});

            m_mSDF_box = m_sdf_box.get_box(m_sdf_resolution, m_slide_x, m_slide_y, m_slide_z);

            m_sdf_tree->root() = m_sdf_root;

            m_timer.start();
            m_mSDF = m_sdf_tree->polygonize(m_sdf_resolution, m_sdf_box);
            m_timer.stop();

            m_ipolytms = m_timer.ms();
            m_ipolytus = m_timer.us();

            center_camera(*m_mSDF_box);
        }

        m_sdf_node = nullptr;
        m_node_1_selection = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Tree"))
    {
        m_sdf_root = nullptr;
        m_sdf_node = nullptr;
        m_node_1_selection = true;
        m_sdf_tree->root() = nullptr;
        m_mSDF->clear();
    }
}
