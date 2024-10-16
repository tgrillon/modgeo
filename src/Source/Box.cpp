#include "Box.h"

namespace gm
{
    /*!
\class Box box.h
\brief An axis aligned box.

The class stores the opposite two corners as vectors.
The center and the radius (diagonal vector) are computed on the
fly by inline functions.

The vertices of m_a box can be obtained by the Box::vertex()
member function which returns one of the eight vertices of the box.
The two opposite corners can be obtained faster as follows:

\code
Box box(Vector(0.0,0.0,0.0),Vector(1.0,1.0,1.0)); // Unit box
Vector m_a=box[0]; // Lower vertex
Vector m_b=box[1]; // Opposite vertex
\endcode
*/

    const float Box::s_epsilon = 1.0e-5; //!< Epsilon value used to check intersections and some round off errors.
    const Box Box::s_null(0.0);           //!< s_null box, equivalent to: \code Box(Vector(0.0)); \endcode

    const int Box::s_edge[24] =
        {
            0, 1, 2, 3, 4, 5, 6, 7,
            0, 2, 1, 3, 4, 6, 5, 7,
            0, 4, 1, 5, 2, 6, 3, 7};

    const Vector Box::s_normal[6] =
        {
            Vector(-1.0, 0.0, 0.0),
            Vector(0.0, -1.0, 0.0),
            Vector(0.0, 0.0, -1.0),
            Vector(1.0, 0.0, 0.0),
            Vector(0.0, 1.0, 0.0),
            Vector(0.0, 0.0, 1.0)};

    /*!
    \brief Create m_a box given m_a center point and the half side length.
    \param c center.
    \param r Half side length.
    */
    Box::Box(const Vector &c, float r)
    {
        m_a = c - Vector(r);
        m_b = c + Vector(r);
    }

    /*!
    \brief Create m_a box given two opposite corners.

    Note that this constructor does not check the coordinates of the two vectors.
    Therefore, the coordinates of m_a should be lower than those of m_b.

    To create the axis aligned bounding box of two vectors m_a and m_b in
    the general case, one should use:
    \code
    Box box(Vector::min(m_a,m_b),Vector::max(m_a,m_b));
    \endcode
    \param a,b End vertices.
    */
    Box::Box(const Vector &a, const Vector &b)
    {
        Box::m_a = a;
        Box::m_b = b;
    }

    /*!
    \brief Create m_a cube centered at the origin and of given half side length.

    This is equivalent to:
    \code
    Box box(Vector(0.0),2.0);  // Simplified constructor Box(2.0);
    \endcode
    \param r Half side length.
    */
    Box::Box(float r)
    {
        m_a = -Vector(r);
        m_b = Vector(r);
    }

    /*!
    \brief Creates the bounding box of m_a set of points.
    \param v Array of vertices.
    */
    Box::Box(const std::vector<Vector> &v)
    {
        for (int j = 0; j < 3; j++)
        {
            m_a(j) = v.at(0)(j);
            m_b(j) = v.at(0)(j);
            for (int i = 1; i < v.size(); i++)
            {
                if (v.at(i)(j) < m_a(j))
                {
                    m_a(j) = v.at(i)(j);
                }
                if (v.at(i)(j) > m_b(j))
                {
                    m_b(j) = v.at(i)(j);
                }
            }
        }
    }

    /*!
    \brief Create m_a box embedding two boxes.
    \param x,y Argument boxes.
    */
    Box::Box(const Box &x, const Box &y)
    {
        m_a = min(x.m_a, y.m_a);
        m_b = max(x.m_b, y.m_b);
    }

    /*!
    \brief Computes the sub-box in the n-th octant.
    \param n Octant index.
    */
    Box Box::sub(int n) const
    {
        Vector c = center();
        return Box(Vector((n & 1) ? c(0) : m_a(0), (n & 2) ? c(1) : m_a(1), (n & 4) ? c(2) : m_a(2)),
                   Vector((n & 1) ? m_b(0) : c(0), (n & 2) ? m_b(1) : c(1), (n & 4) ? m_b(2) : c(2)));
    }

    /*!
    \brief Overloaded.
    \param s Stream.
    \param box The box.
    */
    std::ostream &operator<<(std::ostream &s, const Box &box)
    {
        s << "Box(" << box.m_a << ',' << box.m_b << ")";
        return s;
    }

    /*!
    \brief Translates m_a box.

    \param t Translation vector.
    */
    void Box::translate(const Vector &t)
    {
        m_a += t;
        m_b += t;
    }

    /*!
    \brief Scales m_a box.

    Note that this function handles negative coefficients in
    the scaling vector (by swapping coordinates if need be).
    \param s Scaling.
    */
    void Box::scale(float s)
    {
        m_a *= s;
        m_b *= s;

        // Swap coordinates for negative coefficients
        if (s < 0.0)
        {
            Vector t = m_a;
            m_a = m_b;
            m_b = t;
        }
    }

    Mesh Box::get_box(int resolution, int slide_x, int slide_y, int slide_z) const
    {
        Mesh mesh(GL_LINES);

        get_grid(mesh, resolution, slide_x, 4, 0, 6, 2, 5, 1, 7, 3);
        get_grid(mesh, resolution, slide_y, 2, 3, 6, 7, 0, 1, 4, 5);
        get_grid(mesh, resolution, slide_z, 0, 1, 2, 3, 4, 5, 6, 7);

        return mesh;
    }
    
    void Box::get_grid(Mesh &mesh, int n, int s, int _a, int _b, int _c, int _d, int _e, int _f, int _g, int _h) const
    {
        Point a = Point(vertex(_a));
        Point b = Point(vertex(_b));
        Point c = Point(vertex(_c));
        Point d = Point(vertex(_d));
        Point e = Point(vertex(_e));
        Point f = Point(vertex(_f));
        Point g = Point(vertex(_g));
        Point h = Point(vertex(_h));

        float step = static_cast<float>(s) / n; 

        a = a + step * Vector(a, e);
        b = b + step * Vector(b, f);
        c = c + step * Vector(c, g);
        d = d + step * Vector(d, h);

        Vector ab(a, b);
        Vector ac(a, c);
        Vector cd(c, d);
        Vector bd(b, d);

        step = 1.f / n; 
        for (int i = 0; i <= n; ++i)
        {
            float istep = i * step; 
            mesh.vertex(a + istep * ab);
            mesh.vertex(c + istep * cd);

            for (int j = 0; j <= n; ++j)
            {
                float jstep = j * step; 
                mesh.vertex(a + jstep * ac);
                mesh.vertex(b + jstep * bd);
            }
        }
    }
} // namespace gm
