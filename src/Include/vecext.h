#pragma once

#include "pch.h"

//! gKit vec.h extensions

bool equal(float a, float b);

//! return true if a < b;
bool operator<(const Vector &a, const Vector &b);
//! return true if a > b;
bool operator>(const Vector &a, const Vector &b);
//! return true if a == b;
bool operator==(const Vector &a, const Vector &b);

Vector operator+=(const Vector &a, const Vector &b);
Vector operator*=(const Vector &a, const Vector &b);

//! renvoie le vecteur -u.
vec2 operator-(const vec2 &u);
//! renvoie le vecteur u+v.
vec2 operator+(const vec2 &u, const vec2 &v);
//! renvoie le vecteur u-v.
vec2 operator-(const vec2 &u, const vec2 &v);
//! renvoie le vecteur k*u;
vec2 operator*(const float k, const vec2 &v);
//! renvoie le vecteur k*v;
vec2 operator*(const vec2 &v, const float k);
//! renvoie le vecteur (a.x*b.x, a.y*b.y, a.z*b.z ).
vec2 operator*(const vec2 &a, const vec2 &b);
//! renvoie le vecteur v/k;
vec2 operator/(const vec2 &v, const float k);

Vector min(const Vector &a, const Vector &b);
Vector max(const Vector &a, const Vector &b);

Point operator%(const Point& p, int t);

Vector abs(const Vector &a);
Point abs(const Point &a);
Point round(const Point &a);

vec2 abs(const vec2 &a);
vec2 max(const vec2 &a, float s);

float length(const vec2 &v);
float length2(const vec2 &v);
