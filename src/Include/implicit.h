#pragma once

#include <limits>
#include <cfloat>

#include "vec.h"

struct Ray
{
  Point o;            // origine
  float pad;
  Vector d;           // direction
  float tmax;         // tmax= 1 ou \inf, le rayon est un segment ou une demi droite infinie
  
  Ray( const Point& _o, const Point& _e ) :  o(_o), d(Vector(_o, _e)), tmax(1) {} // segment, t entre 0 et 1
  Ray( const Point& _o, const Vector& _d ) :  o(_o), d(_d), tmax(FLT_MAX) {}  // demi droite, t entre 0 et \inf
  Ray( const Point& _o, const Vector& _d, const float _tmax ) :  o(_o), d(_d), tmax(_tmax) {} // explicite
};

class Implicit
{
public:
  virtual float Value(const Point& p) const;
  virtual bool Inside(const Point& p) const;
  virtual Point Gradient(const Point& p) const;
  virtual bool Intersect(const Ray& ray, float& t) const;
protected:
  float lambda= 1.0;
};