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
  m_camera.projection(window_width(), window_height(), 45);

  m_grid= make_grid(10);

  std::string heightmapPath= std::string(MAP_DIR) + "/heightmap2.jpg";
  // m_hm= mg::Grid::create(10, 10);
  m_hm= mg::Grid::load(heightmapPath, -15.0f);
  // m_hm(5, 5, {0, -30, 0});
  m_bezier= mg::Bezier::create(m_hm);

  m_patch= m_bezier.poligonize(10);

  // Point p= {0, 30, 0};
  // m_bezier.control_point(5, 5, p);

  Point pmin, pmax; 
  m_patch.bounds(pmin, pmax);
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
  m_patch.release();

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
    ImGui::SliderInt("Resolution", &m_resolution, 3, 200);
    if (ImGui::Button("Render"))
    {
      m_patch= m_bezier.poligonize(m_resolution);
    }
    ImGui::End();

    ImGui::Begin("Statistiques");
    auto[cpums, cpuus]= cpu_time();
    auto[gpums, gpuus]= gpu_time();
    ImGui::Text("fps : %.2f ", (1000.f / delta_time()));
    ImGui::Text("cpu : %i ms %i us ", cpums, cpuus);
    ImGui::Text("gpu : %i ms %i us", gpums, gpuus);
    ImGui::Text("total : %.2f ms", delta_time());
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

int Viewer::render_any()
{
  // m_bezier= mg::Bezier::create(5, 5);
  // Point p= {0, (sin(global_time() * 0.002f) * 100), 0};
  // m_bezier.control_point(5, 5, p);

  draw(m_patch, Identity(), m_camera);  

  return 0;
}

int Viewer::render_menu_bar()
{
  ImGui::DockSpace(ImGui::GetID("DockSpace"));

  if (ImGui::BeginMainMenuBar()) 
  {
    if (ImGui::BeginMenu("Edit")) 
    {
      ImGui::MenuItem("Style Editor", NULL, &m_show_style_editor);
      ImGui::MenuItem("Control Panel", NULL, &m_show_ui);
      ImGui::MenuItem("Exit", NULL, &m_exit);
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Style"))
    {
      if (ImGui::MenuItem("Dark"))
      {
        ImGui::StyleColorsDark();
      }
      if (ImGui::MenuItem("Light"))
      {
        ImGui::StyleColorsLight();
      }
      if (ImGui::MenuItem("Classic"))
      {
        ImGui::StyleColorsClassic();
      }

      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }

  // ImGui::ShowDemoWindow();

  return 0;
}
