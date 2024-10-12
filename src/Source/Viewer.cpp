#include "Viewer.h"

Mesh make_grid(const int n= 10)
{
    Mesh grid= Mesh(GL_LINES);
    
    // grille
    grid.color(White());
    for(int x= 0; x < n; x++)
    {
        float px= float(x) - float(n)/2 + .5f;
        grid.vertex(Point(px, 0, - float(n)/2 + .5f)); 
        grid.vertex(Point(px, 0, float(n)/2 - .5f));
    }

    for(int z= 0; z < n; z++)
    {
        float pz= float(z) - float(n)/2 + .5f;
        grid.vertex(Point(- float(n)/2 + .5f, 0, pz)); 
        grid.vertex(Point(float(n)/2 - .5f, 0, pz));  
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

 
Viewer::Viewer() : App(1024, 640), m_framebuffer(window_width(), window_height())
{
}

int Viewer::init_any()
{
    // m_grid= make_grid(10);

    // std::string heightmapPath= std::string(MAP_DIR) + "/heightmap3.jpg";
    // std::vector<Point> curve;

    // curve= gm::curve_points(m_nb_control_points_spline, [](double t) {
    //     return Point(10 * t, 0, 0);
    // });

    // m_line= Mesh(GL_LINE_STRIP);
    // for (const auto& p : curve)
    // {
    //     m_line.vertex(p);
    // }

    // m_spline= gm::Revolution::create(curve);

    // std::chrono::high_resolution_clock::time_point start= std::chrono::high_resolution_clock::now();
    // m_mSpline= m_spline.polygonize(4);
    // std::chrono::high_resolution_clock::time_point stop= std::chrono::high_resolution_clock::now();

    // m_spline_time_polygonize= float(std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());

    // std::vector<std::vector<Point>> surface; 

    // surface= gm::surface_points(m_nb_control_points_patch, [](double u, double v) {
    //   return Point(u * 10., sin(10 * v * 2 * M_PI), v * 10);
    // });

    // m_grid= Mesh(GL_LINES);
    // for (int h= 1; h < m_nb_control_points_patch; ++h)
    // {
    //     for (int w= 1; w < m_nb_control_points_patch; ++w)
    //     {
    //         m_grid.vertex(surface[h][w-1]);
    //         m_grid.vertex(surface[h][w]);

    //         m_grid.vertex(surface[h-1][w]);
    //         m_grid.vertex(surface[h][w]);
    //     }
    // }

    // m_patch= gm::Bezier::create(surface);
    // start= std::chrono::high_resolution_clock::now();
    // m_mPatch= m_patch.polygonize(m_patch_resolution); 
    // stop= std::chrono::high_resolution_clock::now();

    // m_patch_time_polygonize= float(std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());

    // Point pmin, pmax;
    // m_mSpline.bounds(pmin, pmax);
    // m_camera.lookat(pmin, pmax); 


    auto start= std::chrono::high_resolution_clock::now();
    m_teapot.load_pacthes(std::string(DATA_DIR) + "/teapot");
    m_mTeapot= m_teapot.polygonize(4);
    auto stop= std::chrono::high_resolution_clock::now();

    m_patch_time_polygonize= float(std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());

    Point pmin, pmax;
    m_mTeapot.bounds(pmin, pmax);
    m_camera.lookat(pmin, pmax);

    return 0;
}

int Viewer::init_imgui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io= ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Dear ImGui style
    // ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    m_expr_patch= exprtk_wrapper_init();
    add_double_variable(m_expr_patch, "u");
    add_double_variable(m_expr_patch, "v");
    set_expression_count(m_expr_patch, 3);

    m_expr_spline= exprtk_wrapper_init();
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
    m_grid.release();
    m_mPatch.release();
    m_mSpline.release();
    m_mTeapot.release();

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
    Transform model= Identity();
    Transform view= m_camera.view();
    Transform projection= m_camera.projection();

    DrawParam param;
    param.model(model).view(view).projection(projection);


    if (m_show_spline)
    {
        if (m_spline_edges_only)
        {
            glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
        }
        else if (m_spline_points_only)
        {
            glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);

        }
        else 
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        // param.draw(m_mSpline);

        // if (m_show_spline_curve)
        // {
            // param.draw(m_line);
        // }
    }
    else if (m_show_patch)
    {
        if (m_patch_edges_only)
        {
            glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
        }
        else if (m_patch_points_only)
        {
            glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);

        }
        else 
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        // param.draw(m_mPatch);

        // if (m_show_patch_grid)
        // {
            // param.draw(m_grid);
        // }
        param.draw(m_mTeapot);
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
        ImGuiIO& io= ImGui::GetIO(); (void)io;

        io.WantCaptureMouse= false; 
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
        ImVec2(1, 0)
    );

    ImGui::End(); 
 
    if (m_show_ui)
    {
        ImGui::Begin("Parameters");
        ImVec2 sz = ImVec2(-FLT_MIN, 45.0f);
        if (m_show_spline)
        {
            if (ImGui::Button("Patch Demo", sz))
            {
                m_show_patch= true; 
                m_show_spline= false;
                Point pmin, pmax;
                m_mPatch.bounds(pmin, pmax);
                m_camera.lookat(pmin, pmax); 
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
                ImGui::SetTooltip("Activate patch demo.");
            ImGui::BeginDisabled();
            ImGui::Button("Spline Demo", sz);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
                ImGui::SetTooltip("Spline demo is active.");
            ImGui::EndDisabled();
        }
        else 
        {
            ImGui::BeginDisabled();
            ImGui::Button("Patch Demo", sz);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
                ImGui::SetTooltip("Patch demo is active.");
            ImGui::EndDisabled();
            if (ImGui::Button("Spline Demo", sz))
            {
                m_show_patch= false; 
                m_show_spline= true;
                Point pmin, pmax;
                m_mSpline.bounds(pmin, pmax);
                m_camera.lookat(pmin, pmax); 
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
                ImGui::SetTooltip("Activate spline demo.");
        }
        if (m_show_patch)
        {
            ImGui::SeparatorText("Bézier Patch");
            ImGui::PushID(1);
            ImGui::SliderInt("Resolution", &m_patch_resolution, 4, 1000);
            ImGui::PopID();
            ImGui::PushID(1);
            ImGui::SliderInt("#Control points", &m_nb_control_points_patch, 4, 31);
            ImGui::PopID();
            ImGui::Text("You can use the two variables 'u' and 'v' in each expression below :");
            ImGui::PushID(1);
            ImGui::InputText("X", surface_function_input_x, IM_ARRAYSIZE(surface_function_input_x));
            ImGui::PopID();
            ImGui::PushID(1);
            ImGui::InputText("Y", surface_function_input_y, IM_ARRAYSIZE(surface_function_input_y));
            ImGui::PopID();
            ImGui::PushID(1);
            ImGui::InputText("Z", surface_function_input_z, IM_ARRAYSIZE(surface_function_input_z));
            ImGui::PopID();
            ImGui::PushID(1);
            ImGui::Checkbox("Edges only", &m_patch_edges_only);
            ImGui::PopID();
            ImGui::SameLine();
            ImGui::PushID(1);
            ImGui::Checkbox("Points only", &m_patch_points_only);
            ImGui::PopID();
            ImGui::SameLine();
            ImGui::PushID(1);
            ImGui::Checkbox("Grid", &m_show_patch_grid);
            ImGui::PopID();
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
                surface = gm::surface_points(m_nb_control_points_patch, [&](double u_val, double v_val) {
                    set_double_variable_value(m_expr_patch, "u", u_val);
                    set_double_variable_value(m_expr_patch, "v", v_val);
                    double x = get_evaluated_value(m_expr_patch, 0); 
                    double y = get_evaluated_value(m_expr_patch, 1); 
                    double z = get_evaluated_value(m_expr_patch, 2); 
                    return Point(x, y, z); 
                });

                m_grid.clear();
                for (int h= 1; h < m_nb_control_points_patch; ++h)
                {
                    for (int w= 1; w < m_nb_control_points_patch; ++w)
                    {
                        m_grid.vertex(surface[h][w-1]);
                        m_grid.vertex(surface[h][w]);

                        m_grid.vertex(surface[h-1][w]);
                        m_grid.vertex(surface[h][w]);
                    }
                }

                m_patch= gm::Bezier::create(surface);
                std::chrono::high_resolution_clock::time_point start= std::chrono::high_resolution_clock::now();
                // m_mPatch= m_patch.polygonize(m_patch_resolution);
                m_mTeapot= m_teapot.polygonize(m_patch_resolution);
                std::chrono::high_resolution_clock::time_point stop= std::chrono::high_resolution_clock::now();

                m_patch_time_polygonize= float(std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());

                // Point pmin, pmax;
                // m_mPatch.bounds(pmin, pmax);
                // m_camera.lookat(pmin, pmax); 
            }
        }

        if (m_show_spline)
        {  
            ImGui::SeparatorText("Bézier Spline");
            ImGui::PushID(1);
            ImGui::SliderInt("Resolution", &m_spline_resolution, 3, 1000);
            ImGui::PopID();
            ImGui::PushID(1);
            ImGui::SliderInt("#Control points", &m_nb_control_points_spline, 2, 31);
            ImGui::PopID();
            ImGui::Text("You can use a variable 't' in each expression below :");
            ImGui::PushID(1);
            ImGui::InputText("X", curve_function_input_x, IM_ARRAYSIZE(curve_function_input_x));
            ImGui::PopID();
            ImGui::PushID(1);
            ImGui::InputText("Y", curve_function_input_y, IM_ARRAYSIZE(curve_function_input_y));
            ImGui::PopID();
            ImGui::PushID(1);
            ImGui::InputText("Z", curve_function_input_z, IM_ARRAYSIZE(curve_function_input_z));
            ImGui::PopID();
            ImGui::PushID(1);
            ImGui::Checkbox("Edges only", &m_spline_edges_only);
            ImGui::PopID();
            ImGui::SameLine();
            ImGui::PushID(1);
            ImGui::Checkbox("Points only", &m_spline_points_only);
            ImGui::PopID();
            ImGui::SameLine();
            ImGui::PushID(1);
            ImGui::Checkbox("Curve", &m_show_spline_curve);
            ImGui::PopID();
            ImGui::Text("You can use the two variables 't' and 'a' (angle value) in the expression below :");
            ImGui::PushID(1);
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
                curve = gm::curve_points(m_nb_control_points_spline, [&](double t_val) {
                    set_double_variable_value(m_expr_spline, "t", t_val);
                    double x = get_evaluated_value(m_expr_spline, 0); 
                    double y = get_evaluated_value(m_expr_spline, 1); 
                    double z = get_evaluated_value(m_expr_spline, 2); 
                    return Point(x, y, z); 
                });

                m_line.clear();
                for (const auto& p : curve)
                {
                    m_line.vertex(p);
                }

                m_spline= gm::Revolution::create(curve);
                m_spline.radial_fun([&](double t_val, double a_val) {
                    set_double_variable_value(m_expr_spline, "t", t_val);
                    set_double_variable_value(m_expr_spline, "a", a_val);
                    return get_evaluated_value(m_expr_spline, 3);
                });

                std::chrono::high_resolution_clock::time_point start= std::chrono::high_resolution_clock::now();
                m_mSpline= m_spline.polygonize(m_spline_resolution);
                std::chrono::high_resolution_clock::time_point stop= std::chrono::high_resolution_clock::now();

                m_spline_time_polygonize= float(std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());

                Point pmin, pmax;
                m_mSpline.bounds(pmin, pmax);
                m_camera.lookat(pmin, pmax); 
            }
            ImGui::PopID();
        }

        ImGui::End();

        ImGui::Begin("Statistiques");
        ImGui::SeparatorText("Performances");
        auto[cpums, cpuus]= cpu_time();
        auto[gpums, gpuus]= gpu_time();
        ImGui::Text("fps : %.2f ", (1000.f / delta_time()));
        ImGui::Text("cpu : %i ms %i us ", cpums, cpuus);
        ImGui::Text("gpu : %i ms %i us", gpums, gpuus);
        ImGui::Text("total : %.2f ms", delta_time());
        if (m_show_spline)
        {
            ImGui::SeparatorText("Spline Geometry");
            ImGui::Text("#Triangle : %i ", m_mSpline.triangle_count());
            ImGui::Text("#Vertex : %i ", m_mSpline.vertex_count());
            ImGui::Text("#Control points : %i ", m_spline.point_count());
            ImGui::Text("Poligonize Time : %.2f ms", m_spline_time_polygonize);
        }
        
        if (m_show_patch)
        {
            ImGui::SeparatorText("Patch Geometry");
            ImGui::Text("#Triangle : %i ", m_mTeapot.triangle_count());
            // ImGui::Text("#Triangle : %i ", m_mPatch.triangle_count());
            ImGui::Text("#Vertex : %i ", m_mTeapot.vertex_count());
            // ImGui::Text("#Vertex : %i ", m_mPatch.vertex_count());
            ImGui::Text("#Control points : %i ", m_patch.point_count());
            ImGui::Text("Poligonize Time : %.2f ms", m_patch_time_polygonize);
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
