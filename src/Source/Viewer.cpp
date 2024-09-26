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


Viewer::Viewer() : App(1024, 640)
{
}

int Viewer::init()
{
  if (init_imgui() < 0) return -1;

  /********************************/
  /************INIT HERE***********/

  m_camera.projection(window_width(), window_height(), 45);

  m_grid= make_grid(10);

  m_bezier= mg::Bezier::Create(10, 10);

  // Point p= {0, 30, 0};
  // m_bezier.SetControlPoint(5, 5, p);

  m_patch= m_bezier.Poligonize(100);

  Point pmin, pmax; 
  m_patch.bounds(pmin, pmax);
  m_camera.lookat(pmin, pmax);

  /********************************/
  /********************************/

  if (init_sdl_gl() < 0) return -1;

  return 0;
}

int Viewer::render()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  render_ui();

  /********************************/
  /**********RENDER HERE***********/

  m_bezier= mg::Bezier::Create(10, 10);
  Point p= {0, (sin(global_time() * 0.002) * 100), 0};
  m_bezier.SetControlPoint(5, 5, p);

  m_patch= m_bezier.Poligonize(m_resolution);

  draw(m_patch, Identity(), m_camera);  

  /********************************/
  /********************************/

  return 1;
}

int Viewer::quit()
{
  m_grid.release();
  m_patch.release();

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  SDL_Quit();

  return 0;
}

int Viewer::init_sdl_gl()
{
  glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
  
  glClearDepth(1.f);                          // profondeur par defaut
  glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
  glEnable(GL_DEPTH_TEST);                    // activer le ztest

  return 0;
}

int Viewer::init_imgui()
{
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  io= ImGui::GetIO(); (void)io;

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  // Enable docking
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // Setup Platform/Renderer bindings
  // window is the SDL_Window*
  // context is the SDL_GLContext
  ImGui_ImplSDL2_InitForOpenGL(m_window, m_context);
  ImGui_ImplOpenGL3_Init();

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  return 0;
}

void Viewer::render_ui()
{
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  // This is all that needs to be added for this:
  ImGui::DockSpaceOverViewport();

  init_control_panel();

  if (m_show_ui)
  {
    ImGui::Begin("Control Panel", &m_show_ui);
    ImGui::SliderInt("Resolution", &m_resolution, 3, 150);
    ImGui::End();
  }

  // Rendering
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Viewer::init_control_panel()
{
  ImGui::DockSpace(ImGui::GetID("DockSpace"));

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Exit")) {
        m_exit= true;
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      ImGui::MenuItem(
        "Control Panel", nullptr, &m_show_ui
      );
      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
}
