#pragma once

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include "wavefront.h"
#include "texture.h"

#include "orbiter.h"
#include "draw.h"        
#include "app_camera.h"        // classe Application a deriver
#include "uniforms.h"

#include "Bezier.h"

class Application : public AppCamera
{
public:
  Application();
  
  int init();
  int quit();
  int render();

protected:
  Mesh m_objet;
  Mesh m_cube;
  Mesh m_robot;
  Mesh m_repere;

  mg::Bezier m_bezier; 

  double m_deltaTime= 0.0; 
  
  unsigned int m_program;
};
