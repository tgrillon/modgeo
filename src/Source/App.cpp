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
  
  // bool ctrlKey = key_state(SDLK_LCTRL) || key_state(SDLK_RCTRL);
  // auto& io = ImGui::GetIO();

  // if (!ctrlKey && !io.WantCaptureMouse && !io.WantCaptureKeyboard) 
  // {
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
  // }

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
  
  while(events(m_window))
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

