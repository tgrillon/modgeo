#pragma once

#include "pch.h"

#include "Box.h"
// #include "mathematic.h"

/*
SDF point -> line AB

h= min(1, max(0, dot(ap, ab)/dot(ab, ab)))
d= length(P - A - h * ab)

*/

namespace gm
{
    struct Ray
    {
        Point origin { 0, 0, 0 }; 
        Vector direction { 0, 0, 0 }; 

        Point point(double t);
    };

    class Implicit
    {
    public:
        virtual double value(const Vector &p) const;
        virtual bool inside(const Vector &p) const;
        virtual Vector gradient(const Vector &p) const;
        virtual bool intersect(const Ray &ray, double &t) const;

    protected:
        double lambda = 1.0;
    };

    class AnalyticScalarField : public Implicit
    {
    public:
        AnalyticScalarField();
        double value(const Vector &) const override;
        Vector gradient(const Vector &) const override;

        // Normal
        virtual Vector normal(const Vector &) const;

        // dichotomy
        Vector dichotomy(Vector, Vector, double, double, double, const double & = 1.0e-4) const;

        virtual Mesh polygonize(int, const Box &, const double & = 1e-4) const;

    protected:
        static const double s_epsilon; //!< Epsilon value for partial derivatives
    protected:
        static int m_triangle_table[256][16]; //!< Two dimensionnal array storing the straddling edges for every marching cubes configuration.
        static int m_edge_table[256];         //!< Array storing straddling edges for every marching cubes configuration.
    };
} // namespace gm
