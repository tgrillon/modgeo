#include "Bezier.h"

namespace mg
{
Bezier::Bezier() : m_controlPoints()
{
}

void Bezier::SetControlPoint(int r, int c, const Point &point)
{
  assert(r * m_patchWidth + c < m_controlPoints.size());
  m_controlPoints[r * m_patchWidth + c]= point;
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
      int nextI= i + 1;
      int nextJ= j + 1;
      mesh.triangle(i * n + j, nextI * n + j, nextI * n + nextJ);
      mesh.triangle(i * n + j, nextI * n + nextJ, i * n + nextJ);
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
  bz.m_patchWidth= n; 
  bz.m_patchHeight= m;
  for (int i= 0; i < n; ++i)
    for (int j= 0; j < m; ++j)
    {
      bz.m_controlPoints.emplace_back(i, 0, j);
    }

  return bz; 
}

Bezier Bezier::Create(int n, int m, double dt)
{
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<float> dist(-0, 10); 

  Bezier bz; 
  bz.m_patchWidth= n; 
  bz.m_patchHeight= m;
  for (int i= 0; i < n; ++i)
    for (int j= 0; j < m; ++j)
    {
      bz.m_controlPoints.emplace_back(i, sin(dt + i) / (dt + i), j);
    }

  return bz; 
}

Point Bezier::GetPoint(double u, double v) const
{
  Point p; 
  for (int r= 0; r < m_patchHeight; ++r)
  {
    for (int c= 0; c < m_patchWidth; ++c)
    {
      p= p + Bernstein(u, c, m_patchWidth-1) * Bernstein(v, r, m_patchHeight-1) * m_controlPoints[r * m_patchWidth + c]; 
    }
  }

  return p; 
}

double Bezier::Bernstein(double u, int i, int m) const
{
  return bCoeffs[m][i] * pow(u, i) * pow(1 - u, m - i);
}

} // namespace mg

