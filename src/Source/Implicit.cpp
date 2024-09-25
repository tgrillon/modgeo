#include "Implicit.h"

float Implicit::Value(const Point &p) const
{ 
  return std::numeric_limits<float>::infinity(); 
}

bool Implicit::Inside(const Point &p) const
{ 
  return Value(p) < 0.f; 
}

Point Implicit::Gradient(const Point &p) const
{
  return Point();
}

bool Implicit::Intersect(const Ray &ray, float &t) const
{
  return false;
}
