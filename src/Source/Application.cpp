#include "Application.h"

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

Mesh read_mesh(const std::string &objPath)
{
  return read_mesh(objPath.c_str());
}

Application::Application()  : AppCamera(1024, 640) 
{
}

int Application::init()
{
  std::string objPath= std::string(OBJ_DIR) + "/cube.obj";
  m_objet= read_mesh(objPath);

  m_repere= make_grid();

  Point pmin, pmax; 
  m_objet.bounds(pmin, pmax);
  camera().lookat(pmin, pmax);

  glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
  
  glClearDepth(1.f);                          // profondeur par defaut
  glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
  glEnable(GL_DEPTH_TEST);                    // activer le ztest

 return 0;   // pas d'erreur, sinon renvoyer -1
}
 
int Application::quit()
{
  m_repere.release();
  m_objet.release();
  return 0;   // pas d'erreur
}

int Application::render()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  draw(m_objet, Identity(), camera());  
  draw(m_repere, Identity(), camera());  

  return 1;
}
