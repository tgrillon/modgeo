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

Vector min(const Vector &a, const Vector &b)
{
    return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) };
}

Vector max(const Vector &a, const Vector &b)
{
    return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) };
}