#include "pch.h"

#include "vecext.h"

bool equal(float a, float b)
{
    return fabs(a - b) < 1e-3;
}

bool operator<(const Vector &a, const Vector &b)
{
    return a.x < b.x && a.y < b.y && a.z < b.z;
}

bool operator>(const Vector &a, const Vector &b)
{
    return a.x > b.x && a.y > b.y && a.z > b.z;
}

bool operator==(const Vector &a, const Vector &b)
{
    return equal(a.x, b.x) && equal(a.y, b.y) && equal(a.z, b.z);
}

Vector operator+=(const Vector &a, const Vector &b)
{
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

Vector operator*=(const Vector &a, const Vector &b)
{
    return { a.x * b.x, a.y * b.y, a.z * b.z };
}

vec2 operator-(const vec2 &u)
{
    return { -u.x, -u.y };
}

vec2 operator+(const vec2 &u, const vec2 &v)
{
    return { u.x + v.x, u.y + v.y };
}

vec2 operator-(const vec2 &u, const vec2 &v)
{
    return u + (-v);
}

vec2 operator*(const float k, const vec2 &v)
{
    return { k * v.x, k * v.y };
}

vec2 operator*(const vec2 &v, const float k)
{
    return k * v;
}

vec2 operator*(const vec2 &a, const vec2 &b)
{
    return { a.x * b.x, a.y * b.y };
}

vec2 operator/(const vec2 &v, const float k)
{
    assert(k > 0. || k < 0.);
    return { v.x / k, v.y / k };
}

Vector min(const Vector &a, const Vector &b)
{
    return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) };
}

Vector max(const Vector &a, const Vector &b)
{
    return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) };
}

Vector abs( const Vector& a )
{
    return { abs(a(0)), abs(a(1)), abs(a(2)) };
}

Point abs( const Point& a )
{
    return { abs(a(0)), abs(a(1)), abs(a(2)) };
}

vec2 abs(const vec2 &a)
{
    return { abs(a.x), abs(a.y) };
}

vec2 max(const vec2 &a, float s)
{
    return { std::max(a.x, s), std::max(a.y, s) }; 
}

float length2(const vec2 &v)
{
    return v.x * v.x + v.y * v.y;
}

float length(const vec2 &v)
{
    return sqrt(length2(v));
}
