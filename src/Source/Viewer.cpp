#include "Viewer.h"

#include <exprtk.hpp>

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
    m_grid= make_grid(10);

    std::string heightmapPath= std::string(MAP_DIR) + "/heightmap3.jpg";
    std::vector<Point> points;

    points= gm::curve_points(10, [](double t) {
        return Point(10 * cos(t * 2 * M_PI), 10 * sin(t * 2 * M_PI), 10 * t * 2 * M_PI);
    });

    m_spline= gm::Spline::create(points);
    m_mSpline= m_spline.polygonize(m_spline_resolution);

    std::vector<std::vector<Point>> surface; 

    surface= gm::surface_points(10, [](double u, double v) {
      return Point(u * 10., sin(10 * v * 2 * M_PI), v * 10);
    });

    m_patch= gm::Patch::create(surface);
    m_mPatch= m_patch.polygonize(m_patch_resolution); 

    Point pmin, pmax;
    m_mSpline.bounds(pmin, pmax);
    m_camera.lookat(pmin, pmax); 

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
        param.draw(m_mSpline);
    }
    else if (m_show_patch)
    {
        param.draw(m_mPatch);
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
        ImVec2 sz = ImVec2(-FLT_MIN, 0.0f);
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
        ImGui::SeparatorText("Bézier Patch");
        ImGui::PushID(0);
        ImGui::SliderInt("Resolution", &m_patch_resolution, 3, 1000);
        ImGui::PopID();
        ImGui::PushID(1);
        ImGui::InputText("X", surface_function_input_x, IM_ARRAYSIZE(surface_function_input_x));
        ImGui::PopID();
        ImGui::PushID(2);
        ImGui::InputText("Y", surface_function_input_y, IM_ARRAYSIZE(surface_function_input_y));
        ImGui::PopID();
        ImGui::PushID(3);
        ImGui::InputText("Z", surface_function_input_z, IM_ARRAYSIZE(surface_function_input_z));
        ImGui::PopID();
        ImGui::PushID(4);
        ImGui::Checkbox("Edges only", &m_patch_edges_only);
        if (ImGui::Button("Render"))
        {
            exprtk::symbol_table<double> symbol_table;
            double u = 0, v = 0;
            symbol_table.add_variable("u", u);
            symbol_table.add_variable("v", v);
            symbol_table.add_constants();

            std::vector<std::vector<Point>> surface; 
            exprtk::parser<double> parser;

            exprtk::expression<double> expression_x;
            expression_x.register_symbol_table(symbol_table);
            if (!parser.compile(surface_function_input_x, expression_x))
            {
                std::cout << "Error in function parsing : surface_function_input_x" << std::endl;
                exit(1);
            }

            exprtk::expression<double> expression_y;
            expression_y.register_symbol_table(symbol_table);
            if (!parser.compile(surface_function_input_y, expression_y))
            {
                std::cout << "Error in function parsing : surface_function_input_y" << std::endl;
                exit(1);
            }

            exprtk::expression<double> expression_z;
            expression_z.register_symbol_table(symbol_table);
            if (!parser.compile(surface_function_input_z, expression_z))
            {
                std::cout << "Error in function parsing : surface_function_input_z" << std::endl;
                exit(1);
            }

            surface = gm::surface_points(10, [&expression_x, &expression_y, &expression_z, &u, &v](double u_val, double v_val) {
                u= u_val;
                v= v_val;
                double x = expression_x.value(); 
                double y = expression_y.value(); 
                double z = expression_z.value(); 
                return Point(x, y, z); 
            });

            m_patch= gm::Patch::create(surface);
            std::chrono::high_resolution_clock::time_point start= std::chrono::high_resolution_clock::now();
            m_mPatch= m_patch.polygonize(m_patch_resolution, m_patch_edges_only ? GL_LINE_STRIP : GL_TRIANGLES);
            std::chrono::high_resolution_clock::time_point stop= std::chrono::high_resolution_clock::now();

            m_patch_time_polygonize= float(std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());

            Point pmin, pmax;
            m_mPatch.bounds(pmin, pmax);
            m_camera.lookat(pmin, pmax); 
        }
        ImGui::PopID();

        ImGui::SeparatorText("Bézier Spline");
        ImGui::PushID(5);
        ImGui::SliderInt("Resolution", &m_spline_resolution, 3, 1000);
        ImGui::PopID();
        ImGui::PushID(6);
        ImGui::InputText("X", spline_function_input_x, IM_ARRAYSIZE(spline_function_input_x));
        ImGui::PopID();
        ImGui::PushID(7);
        ImGui::InputText("Y", spline_function_input_y, IM_ARRAYSIZE(spline_function_input_y));
        ImGui::PopID();
        ImGui::PushID(8);
        ImGui::InputText("Z", spline_function_input_z, IM_ARRAYSIZE(spline_function_input_z));
        ImGui::PopID();
        ImGui::PushID(9);
        ImGui::Checkbox("Edges only", &m_spline_edges_only);
        ImGui::PopID();
        ImGui::PushID(10);
        ImGui::InputText("Radial Function", spline_radial_function_input, IM_ARRAYSIZE(spline_radial_function_input));
        if (ImGui::Button("Render"))
        {
            exprtk::symbol_table<double> symbol_table;
            double t= 0;
            symbol_table.add_variable("t", t);
            symbol_table.add_constants();

            std::vector<Point> curve; 
            exprtk::parser<double> parser;

            exprtk::expression<double> expression_x;
            expression_x.register_symbol_table(symbol_table);
            if (!parser.compile(spline_function_input_x, expression_x))
            {
                std::cout << "Error in function parsing : spline_function_input_x" << std::endl;
                exit(1);
            }

            exprtk::expression<double> expression_y;
            expression_y.register_symbol_table(symbol_table);
            if (!parser.compile(spline_function_input_y, expression_y))
            {
                std::cout << "Error in function parsing : spline_function_input_y" << std::endl;
                exit(1);
            }

            exprtk::expression<double> expression_z;
            expression_z.register_symbol_table(symbol_table);
            if (!parser.compile(spline_function_input_z, expression_z))
            {
                std::cout << "Error in function parsing : spline_function_input_z" << std::endl;
                exit(1);
            }

            exprtk::symbol_table<double> symbol_table_r;
            double u= 0, theta= 0;
            symbol_table.add_variable("u", u);
            symbol_table.add_variable("theta", theta);
            symbol_table.add_constants();

            exprtk::expression<double> expression_r;
            expression_r.register_symbol_table(symbol_table);
            if (!parser.compile(spline_radial_function_input, expression_r))
            {
                std::cout << "Error in function parsing : spline_radial_function_input" << std::endl;
                exit(1);
            }

            curve = gm::curve_points(10, [&expression_x, &expression_y, &expression_z, &t](double t_val) {
                t= t_val;
                double x = expression_x.value(); 
                double y = expression_y.value(); 
                double z = expression_z.value(); 
                return Point(x, y, z); 
            });

            m_spline= gm::Spline::create(curve);
            m_spline.radial_fun([&expression_r, &u, &theta](double u_val, double theta_val) {
                u= u_val; 
                theta= theta_val;
                return expression_r.value();
            });

            std::chrono::high_resolution_clock::time_point start= std::chrono::high_resolution_clock::now();
            m_mSpline= m_spline.polygonize(m_spline_resolution, m_spline_edges_only ? GL_LINE_STRIP : GL_TRIANGLES);
            std::chrono::high_resolution_clock::time_point stop= std::chrono::high_resolution_clock::now();

            m_spline_time_polygonize= float(std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());

            Point pmin, pmax;
            m_mSpline.bounds(pmin, pmax);
            m_camera.lookat(pmin, pmax); 
        }
        ImGui::PopID();

        ImGui::End();

        ImGui::Begin("Statistiques");
        ImGui::SeparatorText("Performances");
        auto[cpums, cpuus]= cpu_time();
        auto[gpums, gpuus]= gpu_time();
        ImGui::Text("fps : %.2f ", (1000.f / delta_time()));
        ImGui::Text("cpu : %i ms %i us ", cpums, cpuus);
        ImGui::Text("gpu : %i ms %i us", gpums, gpuus);
        ImGui::Text("total : %.2f ms", delta_time());
        ImGui::SeparatorText("Spline Geometry");
        ImGui::Text("#Triangle : %i ", m_mSpline.triangle_count());
        ImGui::Text("#Vertex : %i ", m_mSpline.vertex_count());
        ImGui::Text("#Control points : %i ", m_spline.point_count());
        ImGui::Text("Poligonize Time : %.2f ms", m_spline_time_polygonize);
        ImGui::SeparatorText("Patch Geometry");
        ImGui::Text("#Triangle : %i ", m_mPatch.triangle_count());
        ImGui::Text("#Vertex : %i ", m_mPatch.vertex_count());
        ImGui::Text("#Control points : %i ", m_patch.point_count());
        ImGui::Text("Poligonize Time : %.2f ms", m_patch_time_polygonize);
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
