#include "App.h"

App::App(const int width, const int height, const int major, const int minor, const int samples) 
{
  m_window= create_window(width, height, major, minor, samples);
  m_context= create_context(m_window);
}

App::~App()
{
  if(m_context) release_context(m_context);
  if(m_window) release_window(m_window);
}

int App::prerender()
{
  // recupere les mouvements de la souris
  int mx, my;
  unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
  int mousex, mousey;
  SDL_GetMouseState(&mousex, &mousey);
  
  ImGuiIO& io = ImGui::GetIO(); (void)io;

  if (!io.WantCaptureMouse) 
  {
    // deplace la camera
    if(mb & SDL_BUTTON(1))
      m_camera.rotation(mx, my);      // tourne autour de l'objet
    else if(mb & SDL_BUTTON(3))
      m_camera.translation((float) mx / (float) window_width(), (float) my / (float) window_height()); // deplace le point de rotation
    else if(mb & SDL_BUTTON(2))
      m_camera.move(mx);           // approche / eloigne l'objet
    
    SDL_MouseWheelEvent wheel= wheel_event();
    if(wheel.y != 0)
    {
      clear_wheel_event();
      m_camera.move(8.f * wheel.y);  // approche / eloigne l'objet
    }
  }

  const char *orbiter_filename= "app_orbiter.txt";
  // copy / export / write orbiter
  if(key_state('c'))
  {
    clear_key_state('c');
    m_camera.write_orbiter(orbiter_filename);
  }
  // paste / read orbiter
  if(key_state('v'))
  {
    clear_key_state('v');
    
    Orbiter tmp;
    if(tmp.read_orbiter(orbiter_filename) < 0)
      // ne pas modifer la camera en cas d'erreur de lecture...
      tmp= m_camera;
    
    m_camera= tmp;
  }
  
  // screenshot
  if(key_state('s'))
  {
    static int calls= 1;
    clear_key_state('s');
    screenshot("app", calls++);
  }
  
  // appelle la fonction update() de la classe derivee
  return update(global_time(), delta_time());
}

int App::run()
{
  if(init() < 0)
    return -1;
  
  glViewport(0, 0, window_width(), window_height());
  
  while(events(m_window) && !m_exit)
  {
    if(prerender() < 0)
      break;
    
    if(render() < 1)
      break;
    
    SDL_GL_SwapWindow(m_window);
    
    if(sync)
      glFinish();
  }
  
  if(quit() < 0)
    return -1;
  
  return 0;
}

bool App::event_imgui()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    // Forward to Imgui
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT
      || (event.type == SDL_WINDOWEVENT
      && event.window.event == SDL_WINDOWEVENT_CLOSE
      && event.window.windowID == SDL_GetWindowID(m_window))) {
      return false;
    }
  }

  return true;
}

void App::vsync_off()
{
  // desactive vsync pour les mesures de temps
  SDL_GL_SetSwapInterval(0);
  printf("sync + vsync  OFF...\n");
  
  // desactive aussi la synchro cpu / gpu...
  sync= false;
}

int App::init()
{
  if (init_imgui() < 0) return -1;
  if (init_any() < 0) return -1;
  if (init_gl() < 0) return -1;

  return 0;
}

int App::init_imgui()
{
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

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

  return 0;
}

int App::init_gl()
{
  glClearColor(0.2f, 0.2f, 0.2f, 1.f);      
  
  glClearDepth(1.f);                        
  glDepthFunc(GL_LESS);                     
  glEnable(GL_DEPTH_TEST);                  

  return 0;
}

int App::quit()
{
  if (quit_any() < 0) return -1;
  if (quit_imgui() < 0) return -1;
  if (quit_sdl() < 0) return -1;

  return 0;
}

int App::quit_sdl()
{
  // Cleanup SDL
  SDL_GL_DeleteContext(m_context);
  SDL_DestroyWindow(m_window);
  SDL_Quit();

  return 0;
}

int App::quit_imgui()
{
  // Cleanup ImGui
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  return 0;
}
