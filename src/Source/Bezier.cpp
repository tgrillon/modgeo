#include "Bezier.h"

namespace mg
{
Bezier::Bezier() : m_control_points()
{
}

void Bezier::SetControlPoint(int r, int c, const Point &point)
{
  assert(r * m_patch_width + c < m_control_points.size());
  m_control_points[r * m_patch_width + c]= point;
}

Mesh Bezier::Poligonize(int n) const
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
      Point p= GetPoint(u, v);
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

Bezier Bezier::Create(int n, int m)
{
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<float> dist(-0, 10); 

  Bezier bz; 
  bz.m_patch_width= n; 
  bz.m_patch_height= m;
  for (int i= 0; i < n; ++i)
    for (int j= 0; j < m; ++j)
    {
      bz.m_control_points.emplace_back(i, 0, j);
    }

  return bz; 
}

Bezier Bezier::Create(int n, int m, double dt)
{
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<float> dist(-0, 10); 

  Bezier bz; 
  bz.m_patch_width= n; 
  bz.m_patch_height= m;
  for (int i= 0; i < n; ++i)
    for (int j= 0; j < m; ++j)
    {
      bz.m_control_points.emplace_back(i, sin(dt + i) / (dt + i), j);
    }

  return bz; 
}

Point Bezier::GetPoint(double u, double v) const
{
  Point p; 
  for (int r= 0; r < m_patch_height; ++r)
  {
    for (int c= 0; c < m_patch_width; ++c)
    {
      p= p + Bernstein(u, c, m_patch_width-1) * Bernstein(v, r, m_patch_height-1) * m_control_points[r * m_patch_width + c]; 
    }
  }

  return p; 
}

double Bezier::Bernstein(double u, int i, int m) const
{
  return binomal_coeffs[m][i] * pow(u, i) * pow(1 - u, m - i);
}

} // namespace mg

