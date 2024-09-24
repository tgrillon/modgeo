#pragma once

#include "wavefront.h"
#include "texture.h"

#include "orbiter.h"
#include "draw.h"        
#include "app_camera.h"        // classe Application a deriver
#include "uniforms.h"

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

  unsigned int m_program;
};
