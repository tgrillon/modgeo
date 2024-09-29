#include "Bezier.h"

#include "image_io.h"

namespace mg
{
Bezier::Bezier() : m_control_points()
{
}

Mesh Bezier::poligonize(int n) const
{
  assert(n > 1);

  Mesh mesh(GL_TRIANGLES); 

  double step= 1.0 / (n - 1);
  for (int i= 0; i < n; ++i)
  { 
    double u= step * i; 
    for (int j= 0; j < n; ++j)
    {
      double v= step * j; 
      Point p= point(u, v);
      mesh.vertex(p);
    }
  }

  for (int i= 0; i < n - 1; ++i)
  { 
    for (int j= 0; j < n - 1; ++j)
    {
      int next_i= i + 1;
      int next_j= j + 1;
      mesh.triangle(i * n + j, next_i * n + j, next_i * n + next_j);
      mesh.triangle(i * n + j, next_i * n + next_j, i * n + next_j);
    }
  }

  return mesh;
}

Bezier Bezier::create(const Grid& grid)
{
  Bezier bz; 
  bz.m_control_points= grid;

  return bz; 
}

// Bezier> Bezier::create(int n, int m, double dt)
// {
//   std::random_device dev;
//   std::mt19937 rng(dev());
//   std::uniform_real_distribution<float> dist(-0, 10); 

//   Bezier bz; 
//   bz.m_patch_width= n; 
//   bz.m_patch_height= m;
//   for (int i= 0; i < n; ++i)
//     for (int j= 0; j < m; ++j)
//     {
//       bz.m_control_points.emplace_back(i, sin(dt + i) / (dt + i), j);
//     }

//   return std::make_shared<Bezier>(bz); 
// }

Point Bezier::point(double u, double v) const
{
  Point p; 
  for (int r= 0; r < height(); ++r)
  {
    for (int c= 0; c < width(); ++c)
    {
      p= p + bernstein(u, c, width()-1) * bernstein(v, r, height()-1) * m_control_points.at(r * width() + c); 
    }
  }

  return p; 
}

double Bezier::bernstein(double u, int i, int m) const
{
  return binomal_coeffs[m][i] * pow(u, i) * pow(1 - u, m - i);
}

Grid Grid::create(unsigned int w, unsigned int h)
{
  Grid g; 
  g.m_width= w;
  g.m_height= h;

  for (int z= 0; z < h; ++z)
    for (int x= 0; x < w; ++x)
      g.m_points.emplace_back(x, 0, z);

  return g;
}

Grid Grid::load(const std::string &height_map, float scale)
{
  Image hm= read_image(height_map.c_str());
  Grid g; 

  g.m_width= hm.width(); 
  g.m_height= hm.height(); 

  for (int z= 0; z < hm.height(); ++z)
    for (int x= 0; x < hm.width(); ++x)
      g.m_points.emplace_back(static_cast<float>(x), scale*hm(x, z).r, static_cast<float>(z));

  return g;
}

void Grid::operator()(unsigned int x, unsigned int y, const Point& value)
{
  assert(x < m_width && y < m_height);

  m_points[y * m_width + x]= value;
}

} // namespace mg
