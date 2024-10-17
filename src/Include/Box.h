// Box

#pragma once

#include "pch.h"

#include "vecext.h"
#include "utils.h"

namespace gm
{

    class Box
    {
    public:
        //! Empty.
        Box() {}
        explicit Box(float);
        explicit Box(const Vector &, const Vector &);
        explicit Box(const Vector &, float);
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

        void update(const Vector &c, float r);
        void a(const Vector &a);
        void b(const Vector &b);

        Vector size() const;
        Vector diagonal() const;
        float radius() const;

        bool inside(const Box &) const;
        bool inside(const Vector &) const;

        float volume() const;
        float area() const;

        // Compute sub-box
        Box sub(int) const;

        // Translation, scale
        void translate(const Vector &);
        void scale(float);

        Ref<Mesh> get_box(int resolution = 1, int slide_x = 0, int slide_y = 0, int slide_z = 0) const;

        friend std::ostream &operator<<(std::ostream &, const Box &);

    private: 
        void get_grid(Ref<Mesh> mesh, int n, int s, const Point& a, const Point& b, const Point& c, const Point& d, const Point& e, const Point& f, const Point& g, const Point& h) const; 

    protected:
        Vector m_a, m_b; //!< Lower and upper vertex.

    public:
        static const float s_epsilon;    //!< Internal \htmlonly\s_epsilon;\endhtmlonly for ray intersection tests.
        static const Box s_null;         //!< Empty box.
        static const int s_edge[24];     //!< Edge vertices.
        static const Vector s_normal[6]; //!< Face normals.
    };
    //! Returns either end vertex of the box.
    inline Vector &Box::operator[](int i)
    {
        if (i == 0)
            return m_a;
        else
            return m_b;
    }

    //! Overloaded.
    inline Vector Box::operator[](int i) const
    {
        if (i == 0)
            return m_a;
        else
            return m_b;
    }

    //! Returns the center of the box.
    inline Vector Box::center() const
    {
        return 0.5 * (m_a + m_b);
    }

    /*!
    \brief Returns the diagonal of the box.
    */
    inline Vector Box::diagonal() const
    {
        return (m_b - m_a);
    }

    /*!
    \brief Compute the size (width, length and height) of m_a box.
    \sa Box::diagonal()
    */
    inline Vector Box::size() const
    {
        return m_b - m_a;
    }

    /*!
    \brief Returns the radius of the box, i.e. the length of the half diagonal of the box.
    */
    inline float Box::radius() const
    {
        return 0.5 * length(m_b - m_a);
    }

    /*!
    \brief Returns the k-th vertex of the box.

    The returned vector is computed by analysing the first three bits of k as follows:
    \code
    Vector vertex=Vector((k&1)?m_b[0]:m_a[0],(k&2)?m_b[1]:m_a[1],(k&4)?m_b[2]:m_a[2]);
    \endcode
    */
    inline Vector Box::vertex(int k) const
    {
        return Vector((k & 1) ? m_b(0) : m_a(0), (k & 2) ? m_b(1) : m_a(1), (k & 4) ? m_b(2) : m_a(2));
    }

    inline void Box::a(const Vector &a)
    {
        m_a = a;
    }

    inline void Box::b(const Vector &b)
    {
        m_b = b;
    }

    inline void Box::update(const Vector &c, float r)
    {
        m_a = c - Vector(r);
        m_b = c + Vector(r);
    }

    //! Compute the volume of m_a box.
    inline float Box::volume() const
    {
        Vector side = m_b - m_a;
        return side(0) * side(1) * side(2);
    }

    /*!
    \brief Compute the surface area of m_a box.
    */
    inline float Box::area() const
    {
        Vector side = m_b - m_a;
        return 2.0 * (side(0) * side(1) + side(0) * side(2) + side(1) * side(2));
    }

    /*!
    \brief Check if an argument box is inside the box.
    \param box The box.
    */
    inline bool Box::inside(const Box &box) const
    {
        return ((m_a < box.m_a) && (m_b > box.m_b));
    }

    /*!
    \brief Check if m_a point is inside the box.
    \param p Point.
    */
    inline bool Box::inside(const Vector &p) const
    {
        return ((m_a < p) && (m_b > p));
    }

    /*!
    \brief Check if two boxes are (strictly) equal.
    \param m_a, m_b Boxes.
    */
    inline int operator==(const Box &m_a, const Box &m_b)
    {
        return (m_a.m_a == m_b.m_a) && (m_a.m_b == m_b.m_b);
    }

    /*!
    \brief Check if two boxes are (strictly) different.
    \param m_a, m_b Boxes.
    */
    inline int operator!=(const Box &m_a, const Box &m_b)
    {
        return !(m_a == m_b);
    }
} // namespace gm
