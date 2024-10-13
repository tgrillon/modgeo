// Box

#pragma once

#include "pch.h"

#include "vecext.h"

namespace gm
{

    class Box
    {
    public:
        //! Empty.
        Box() {}
        explicit Box(double);
        explicit Box(const Vector &, const Vector &);
        explicit Box(const Vector &, double);
        explicit Box(const std::vector<Vector> &);
        explicit Box(const Box &, const Box &);

        //! Empty.
        ~Box() {}

        // Access vertexes
        Vector &operator[](int);
        Vector operator[](int) const;

        // Comparison
        friend int operator==(const Box &, const Box &);
        friend int operator!=(const Box &, const Box &);

        // Acces to vertices
        Vector center() const;
        Vector vertex(int) const;

        Vector size() const;
        Vector diagonal() const;
        double radius() const;

        bool inside(const Box &) const;
        bool inside(const Vector &) const;

        double volume() const;
        double area() const;

        // Compute sub-box
        Box sub(int) const;

        // Translation, scale
        void translate(const Vector &);
        void scale(double);

        friend std::ostream &operator<<(std::ostream &, const Box &);

    protected:
        Vector a, b; //!< Lower and upper vertex.

    public:
        static const double s_epsilon;   //!< Internal \htmlonly\s_epsilon;\endhtmlonly for ray intersection tests.
        static const Box s_null;         //!< Empty box.
        static const int s_edge[24];     //!< Edge vertices.
        static const Vector s_normal[6]; //!< Face normals.
    };
    //! Returns either end vertex of the box.
    inline Vector &Box::operator[](int i)
    {
        if (i == 0)
            return a;
        else
            return b;
    }

    //! Overloaded.
    inline Vector Box::operator[](int i) const
    {
        if (i == 0)
            return a;
        else
            return b;
    }

    //! Returns the center of the box.
    inline Vector Box::center() const
    {
        return 0.5 * (a + b);
    }

    /*!
    \brief Returns the diagonal of the box.
    */
    inline Vector Box::diagonal() const
    {
        return (b - a);
    }

    /*!
    \brief Compute the size (width, length and height) of a box.
    \sa Box::diagonal()
    */
    inline Vector Box::size() const
    {
        return b - a;
    }

    /*!
    \brief Returns the radius of the box, i.e. the length of the half diagonal of the box.
    */
    inline double Box::radius() const
    {
        return 0.5 * length(b - a);
    }

    /*!
    \brief Returns the k-th vertex of the box.

    The returned vector is computed by analysing the first three bits of k as follows:
    \code
    Vector vertex=Vector((k&1)?b[0]:a[0],(k&2)?b[1]:a[1],(k&4)?b[2]:a[2]);
    \endcode
    */
    inline Vector Box::vertex(int k) const
    {
        return Vector((k & 1) ? b(0) : a(0), (k & 2) ? b(1) : a(1), (k & 4) ? b(2) : a(2));
    }

    //! Compute the volume of a box.
    inline double Box::volume() const
    {
        Vector side = b - a;
        return side(0) * side(1) * side(2);
    }

    /*!
    \brief Compute the surface area of a box.
    */
    inline double Box::area() const
    {
        Vector side = b - a;
        return 2.0 * (side(0) * side(1) + side(0) * side(2) + side(1) * side(2));
    }

    /*!
    \brief Check if an argument box is inside the box.
    \param box The box.
    */
    inline bool Box::inside(const Box &box) const
    {
        return ((a < box.a) && (b > box.b));
    }

    /*!
    \brief Check if a point is inside the box.
    \param p Point.
    */
    inline bool Box::inside(const Vector &p) const
    {
        return ((a < p) && (b > p));
    }

    /*!
    \brief Check if two boxes are (strictly) equal.
    \param a, b Boxes.
    */
    inline int operator==(const Box &a, const Box &b)
    {
        return (a.a == b.a) && (a.b == b.b);
    }

    /*!
    \brief Check if two boxes are (strictly) different.
    \param a, b Boxes.
    */
    inline int operator!=(const Box &a, const Box &b)
    {
        return !(a == b);
    }
} // namespace gm
