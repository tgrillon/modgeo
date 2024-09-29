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

  std::string heightmapPath= std::string(MAP_DIR) + "/heightmap4.png";
  m_hm= mg::Grid::load(heightmapPath);
  m_bezier= mg::Bezier::create(m_hm);

  m_patch= m_bezier.poligonize(10);

  // Point p= {0, 30, 0};
  // m_bezier.control_point(5, 5, p);

  // Point pmin, pmax; 
  // m_patch->bounds(pmin, pmax);
  // m_camera.lookat(pmin, pmax);

  return 0;
}


int Viewer::render()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  render_ui();

  m_framebuffer.bind();
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
  glEnable(GL_DEPTH_TEST);

  // render_ui();
  /********************************/
  /**********RENDER HERE***********/

  // m_bezier= mg::Bezier::create(5, 5);
  // Point p= {0, (sin(global_time() * 0.002f) * 100), 0};
  // m_bezier.control_point(5, 5, p);

  // m_patch= m_bezier.poligonize(m_resolution);

  draw(m_patch, Identity(), m_camera);  

  /********************************/
  /********************************/

  m_framebuffer.unbind();

  ImGuiIO& io = ImGui::GetIO(); (void)io;

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // Update and Render additional Platform Windows
  // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
    SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
  }

  return 1;
}

int Viewer::quit_any()
{
  m_grid.release();
  m_patch.release();

  return 0;
}

void Viewer::render_ui()
{
  ImGui::DockSpaceOverViewport();

  init_menu_bar();

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
    ImGui::SliderInt("Resolution", &m_resolution, 3, 150);
    ImGui::End();

    ImGui::Begin("Statistiques");
    ImGui::Text("Frame rate : %.2f ms", delta_time());
    ImGui::Text("FPS : %.2f ", (1000.f / delta_time()));
    ImGui::End();
  }

  if (m_show_style_editor)
  {
    ImGui::Begin("Dear ImGui Style Editor", &m_show_style_editor);
    ImGui::ShowStyleEditor();
    ImGui::End();
  }


  ImGui::Render();
}

void Viewer::init_menu_bar()
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

    ImGui::EndMainMenuBar();
  }
}
