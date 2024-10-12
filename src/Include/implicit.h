#pragma once

#include "pch.h"

#include "Box.h"

struct Ray
{
  Point o;            // origine
  float pad;
  Vector d;           // direction
  float tmax;         // tmax= 1 ou \inf, le rayon est un segment ou une demi droite infinie
  
  Ray(const Point& _o, const Point& _e) :  o(_o), d(Vector(_o, _e)), tmax(1) {} // segment, t entre 0 et 1
  Ray(const Point& _o, const Vector& _d) :  o(_o), d(_d), tmax(FLT_MAX) {}  // demi droite, t entre 0 et \inf
  Ray(const Point& _o, const Vector& _d, const float _tmax) :  o(_o), d(_d), tmax(_tmax) {} // explicite
};

class Implicit
{
public:
  virtual float value(const Point& p) const;
  virtual bool inside(const Point& p) const;
  virtual Point gradient(const Point& p) const;
  virtual bool intersect(const Ray& ray, float& t) const;

protected:
  float lambda= 1.0;
};

class AnalyticScalarField : public Implicit
{
public:
  AnalyticScalarField();
  virtual double value(const Vector&) const;
  virtual Vector gradient(const Vector&) const;

  // Normal
  virtual Vector normal(const Vector&) const;

  // Dichotomy
  Vector dichotomy(Vector, Vector, double, double, double, const double& = 1.0e-4) const;

  virtual void polygonize(int, Mesh&, const Box&, const double& = 1e-4) const;

protected:
  static const double s_epsilon; //!< Epsilon value for partial derivatives
protected:
  static int m_triangle_table[256][16]; //!< Two dimensionnal array storing the straddling edges for every marching cubes configuration.
  static int m_edge_table[256];    //!< Array storing straddling edges for every marching cubes configuration.
};