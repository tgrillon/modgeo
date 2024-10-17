#include "Implicit.h"

namespace gm
{
    const float ImplicitNode::s_epsilon = 0.0001f;
    const int ImplicitNode::s_limit = 10000;

    Point Ray::point(float t) const
    {
        return origin + direction * t;
    }

    /********************** Implicit Node ************************/

    ImplicitNode::ImplicitNode(float lambda, IntersectMethod method) : m_intersect_method(method), m_lambda(lambda)
    {
    }

    float ImplicitNode::value(const Point &p) const
    {
        return FLT_MAX;
    }

    bool ImplicitNode::inside(const Point &p) const
    {
        return value(p) < 0.0;
    }

    Vector ImplicitNode::gradient(const Point &p) const
    {
        //! finite difference solution
        float x = value({p.x + s_epsilon, p.y, p.z}) - value({p.x - s_epsilon, p.y, p.z});
        float y = value({p.x, p.y + s_epsilon, p.z}) - value({p.x, p.y - s_epsilon, p.z});
        float z = value({p.x, p.y, p.z + s_epsilon}) - value({p.x, p.y, p.z - s_epsilon});

        return Vector(x, y, z) / (2 * s_epsilon);
    }

    bool ImplicitNode::intersect(const Ray &ray, float t) const
    {
        switch (m_intersect_method)
        {
        case IntersectMethod::RAY_MARCHING:
            return intersect_ray_marching(ray, t);
        case IntersectMethod::SPHERE_TRACING:
            return intersect_sphere_tracing(ray, t);
        }

        return false;
    }

    void ImplicitNode::intersect_method(IntersectMethod method)
    {
        m_intersect_method = method;
    }

    bool ImplicitNode::intersect_ray_marching(const Ray &ray, float eps) const
    {
        float t = 0.0;
        Point point;
        for (int i = 0; i < s_limit; ++i)
        {
            point = ray.point(t);
            if (value(point) < 0.0)
                return true;
            t += eps;
        }

        return false;
    }

    bool ImplicitNode::intersect_sphere_tracing(const Ray &ray, float eps) const
    {
        float t = 0.0;
        Point point;
        for (int i = 0; i < s_limit; ++i)
        {
            point = ray.point(t);
            float val = std::max(value(point), eps);
            if (val < 0.0)
                return true;
            t += val / m_lambda;
        }

        return false;
    }

    /********************** Implicit Unary Operator ************************/

    ImplicitUnaryOperator::ImplicitUnaryOperator(const Ref<ImplicitNode> &n) : ImplicitNode(), m_node(n)
    {
    }

    /********************** Implicit Hull ************************/

    ImplicitHull::ImplicitHull(const Ref<ImplicitNode>& n, float thickness) : ImplicitUnaryOperator(n), m_thickness(thickness)
    {
    }

    Ref<ImplicitHull> ImplicitHull::create(const Ref<ImplicitNode> &n, float thickness)
    {
        return create_ref<ImplicitHull>(n, thickness);
    }

    float ImplicitHull::value(const Point &p) const
    {
        return abs(m_node->value(p)) - m_thickness * 0.5;
    }

    ImplicitType ImplicitHull::type() const
    {
        return ImplicitType::UNARY_OPERATOR_HULL;
    }

    /********************** Implicit Binary Operator ************************/

    ImplicitBinaryOperator::ImplicitBinaryOperator(const Ref<ImplicitNode> &left, const Ref<ImplicitNode> &right) : ImplicitNode(), m_left(left), m_right(right)
    {
    }

    /*************************** Implicit Union *****************************/

    ImplicitUnion::ImplicitUnion(const Ref<ImplicitNode> &left, const Ref<ImplicitNode> &right) : ImplicitBinaryOperator(left, right)
    {
    }

    Ref<ImplicitUnion> ImplicitUnion::create(const Ref<ImplicitNode> &l, const Ref<ImplicitNode> &r)
    {
        return create_ref<ImplicitUnion>(l, r);
    }

    float ImplicitUnion::value(const Point &p) const
    {
        return std::min(m_left->value(p), m_right->value(p));
    }

    ImplicitType ImplicitUnion::type() const
    {
        return ImplicitType::BINARY_OPERATOR_UNION;
    }

    /************************** Implicit Intersection ****************************/

    ImplicitIntersection::ImplicitIntersection(const Ref<ImplicitNode> &l, const Ref<ImplicitNode> &r) : ImplicitBinaryOperator(l, r)
    {
    }

    Ref<ImplicitIntersection> ImplicitIntersection::create(const Ref<ImplicitNode> &l, const Ref<ImplicitNode> &r)
    {
        return create_ref<ImplicitIntersection>(l, r);
    }

    float ImplicitIntersection::value(const Point &p) const
    {
        return std::max(m_left->value(p), m_right->value(p));
    }

    ImplicitType ImplicitIntersection::type() const
    {
        return ImplicitType::BINARY_OPERATOR_INTERSECTION;
    }

    /************************** Implicit Difference ****************************/

    ImplicitDifference::ImplicitDifference(const Ref<ImplicitNode> &l, const Ref<ImplicitNode> &r) : ImplicitBinaryOperator(l, r)
    {
    }

    Ref<ImplicitDifference> ImplicitDifference::create(const Ref<ImplicitNode> &l, const Ref<ImplicitNode> &r)
    {
        return create_ref<ImplicitDifference>(l, r);
    }

    float ImplicitDifference::value(const Point &p) const
    {
        return std::max(m_left->value(p), -m_right->value(p));
    }

    ImplicitType ImplicitDifference::type() const
    {
        return ImplicitType::BINARY_OPERATOR_DIFFERENCE;
    }

    /************************** Implicit Sphere ****************************/

    ImplicitSphere::ImplicitSphere(const Point &c, float r, float l, IntersectMethod im) : ImplicitNode(l, im), m_center(c), m_radius(r)
    {
    }

    Ref<ImplicitSphere> ImplicitSphere::create(const Point &c, float r, float l, IntersectMethod im)
    {
        return create_ref<ImplicitSphere>(c, r, l, im);
    }

    float ImplicitSphere::value(const Point &p) const
    {
        Vector cp(m_center, p);
        return length(cp) - m_radius;
    }

    ImplicitType ImplicitSphere::type() const
    {
        return ImplicitType::PRIMITIVE_SPHERE;
    }

    /************************** Implicit Box ******************************/

    ImplicitBox::ImplicitBox(const Point &a, const Point &b, float lambda, IntersectMethod im) : ImplicitNode(lambda, im), m_pmin(a), m_pmax(b)
    {
    }

    Ref<ImplicitBox> ImplicitBox::create(const Point &a, const Point &b, float l, IntersectMethod im)
    {
        return create_ref<ImplicitBox>(a, b, l, im);
    }

    float ImplicitBox::value(const Point &p) const
    {
        Vector q = Vector(abs(p) - ((m_pmax - m_pmin) * 0.5));
        return std::min(std::max(q(0), std::max(q(1), q(2))), 0.f) + length(max(q, Vector(0)));
    }

    ImplicitType ImplicitBox::type() const
    {
        return ImplicitType::PRIMITIVE_BOX;
    }

    /************************** Implicit Plane ******************************/

    ImplicitPlane::ImplicitPlane(const Vector &normal, float h, float lambda, IntersectMethod im) : ImplicitNode(lambda, im), m_normal(normal), m_h(h)
    {
    }

    Ref<ImplicitPlane> ImplicitPlane::create(const Vector &normal, float h, float l, IntersectMethod im)
    {
        return std::make_shared<ImplicitPlane>(normal, h, l, im);
    }

    float ImplicitPlane::value(const Point &p) const
    {
        return dot(Vector(p), m_normal) + m_h;
    }

    ImplicitType ImplicitPlane::type() const
    {
        return ImplicitType::PRIMITIVE_PLANE;
    }

    /************************** Implicit Tree ******************************/

    ImplicitTree::ImplicitTree(const Ref<ImplicitNode> &root, float l, IntersectMethod im) : ImplicitNode(l, im), m_root(root)
    {
    }

    Ref<ImplicitTree> ImplicitTree::create(const Ref<ImplicitNode> &root, float l, IntersectMethod im)
    {
        return create_ref<ImplicitTree>(root, l, im);
    }

    float ImplicitTree::value(const Point &p) const
    {
        return m_root->value(p);
    }

    /*!
        \brief Compute the polygonal mesh approximating the implicit surface.

        \param box %Box defining the region that will be polygonized.
        \param n Discretization parameter.
        \param g Returned geometry.
        \param s_epsilon Epsilon value for computing vertices on straddling edges.
        */
    Ref<Mesh> ImplicitTree::polygonize(int n, const Box &box) const
    {
        Ref<Mesh> mesh = create_ref<Mesh>(GL_TRIANGLES);

        int nv = 0;
        const int nx = n;
        const int ny = n;
        const int nz = n;

        Box clipped = box;

        // Clamped integer values
        const int nax = 0;
        const int nbx = nx;
        const int nay = 0;
        const int nby = ny;
        const int naz = 0;
        const int nbz = nz;

        const int size = nx * ny;

        // Intensities
        float *a = new float[size];
        float *b = new float[size];

        // vertex
        Vector *u = new Vector[size];
        Vector *v = new Vector[size];

        // Edges
        int *eax = new int[size];
        int *eay = new int[size];
        int *ebx = new int[size];
        int *eby = new int[size];
        int *ez = new int[size];

        // diagonal of a cell
        Vector d = clipped.diagonal() / (n - 1);

        float za = 0.0;

        // Compute field inside lower Oxy plane
        for (int i = nax; i < nbx; i++)
        {
            for (int j = nay; j < nby; j++)
            {
                u[i * ny + j] = clipped[0] + Vector(i * d(0), j * d(1), za);
                a[i * ny + j] = value(Point(u[i * ny + j]));
            }
        }

        // Compute straddling edges inside lower Oxy plane
        for (int i = nax; i < nbx - 1; i++)
        {
            for (int j = nay; j < nby; j++)
            {
                // We need a xor b, which can be implemented a == !b
                if (!((a[i * ny + j] < 0.0) == !(a[(i + 1) * ny + j] >= 0.0)))
                {
                    auto vertex = dichotomy(u[i * ny + j], u[(i + 1) * ny + j], a[i * ny + j], a[(i + 1) * ny + j], d(0));
                    mesh->vertex(vertex);
                    mesh->normal(normal(vertex));
                    eax[i * ny + j] = nv;
                    nv++;
                }
            }
        }
        for (int i = nax; i < nbx; i++)
        {
            for (int j = nay; j < nby - 1; j++)
            {
                if (!((a[i * ny + j] < 0.0) == !(a[i * ny + (j + 1)] >= 0.0)))
                {
                    auto vertex = dichotomy(u[i * ny + j], u[i * ny + (j + 1)], a[i * ny + j], a[i * ny + (j + 1)], d(1));
                    mesh->vertex(vertex);
                    mesh->normal(normal(vertex));
                    eay[i * ny + j] = nv;
                    nv++;
                }
            }
        }

        // Array for s_edge vertices
        int e[12];

        // For all layers
        for (int k = naz; k < nbz; k++)
        {
            float zb = za + d(2);
            for (int i = nax; i < nbx; i++)
            {
                for (int j = nay; j < nby; j++)
                {
                    v[i * ny + j] = clipped[0] + Vector(i * d(0), j * d(1), zb);
                    b[i * ny + j] = value(Point(v[i * ny + j]));
                }
            }

            // Compute straddling edges inside lower Oxy plane
            for (int i = nax; i < nbx - 1; i++)
            {
                for (int j = nay; j < nby; j++)
                {
                    //   if (((b[i*ny + j] < 0.0) && (b[(i + 1)*ny + j] >= 0.0)) || ((b[i*ny + j] >= 0.0) && (b[(i + 1)*ny + j] < 0.0)))
                    if (!((b[i * ny + j] < 0.0) == !(b[(i + 1) * ny + j] >= 0.0)))
                    {
                        auto vertex = dichotomy(v[i * ny + j], v[(i + 1) * ny + j], b[i * ny + j], b[(i + 1) * ny + j], d(0));
                        mesh->vertex(vertex);
                        mesh->normal(normal(vertex));
                        ebx[i * ny + j] = nv;
                        nv++;
                    }
                }
            }

            for (int i = nax; i < nbx; i++)
            {
                for (int j = nay; j < nby - 1; j++)
                {
                    // if (((b[i*ny + j] < 0.0) && (b[i*ny + (j + 1)] >= 0.0)) || ((b[i*ny + j] >= 0.0) && (b[i*ny + (j + 1)] < 0.0)))
                    if (!((b[i * ny + j] < 0.0) == !(b[i * ny + (j + 1)] >= 0.0)))
                    {
                        auto vertex = dichotomy(v[i * ny + j], v[i * ny + (j + 1)], b[i * ny + j], b[i * ny + (j + 1)], d(1));
                        mesh->vertex(vertex);
                        mesh->normal(normal(vertex));
                        eby[i * ny + j] = nv;
                        nv++;
                    }
                }
            }

            // Create vertical straddling edges
            for (int i = nax; i < nbx; i++)
            {
                for (int j = nay; j < nby; j++)
                {
                    // if ((a[i*ny + j] < 0.0) && (b[i*ny + j] >= 0.0) || (a[i*ny + j] >= 0.0) && (b[i*ny + j] < 0.0))
                    if (!((a[i * ny + j] < 0.0) == !(b[i * ny + j] >= 0.0)))
                    {
                        auto vertex = dichotomy(u[i * ny + j], v[i * ny + j], a[i * ny + j], b[i * ny + j], d(2));
                        mesh->vertex(vertex);
                        mesh->normal(normal(vertex));
                        ez[i * ny + j] = nv;
                        nv++;
                    }
                }
            }

            // Create mesh
            for (int i = nax; i < nbx - 1; i++)
            {
                for (int j = nay; j < nby - 1; j++)
                {
                    int cubeindex = 0;
                    if (a[i * ny + j] < 0.0)
                        cubeindex |= 1;
                    if (a[(i + 1) * ny + j] < 0.0)
                        cubeindex |= 2;
                    if (a[i * ny + j + 1] < 0.0)
                        cubeindex |= 4;
                    if (a[(i + 1) * ny + j + 1] < 0.0)
                        cubeindex |= 8;
                    if (b[i * ny + j] < 0.0)
                        cubeindex |= 16;
                    if (b[(i + 1) * ny + j] < 0.0)
                        cubeindex |= 32;
                    if (b[i * ny + j + 1] < 0.0)
                        cubeindex |= 64;
                    if (b[(i + 1) * ny + j + 1] < 0.0)
                        cubeindex |= 128;

                    // Cube is straddling the surface
                    if ((cubeindex != 255) && (cubeindex != 0))
                    {
                        e[0] = eax[i * ny + j];
                        e[1] = eax[i * ny + (j + 1)];
                        e[2] = ebx[i * ny + j];
                        e[3] = ebx[i * ny + (j + 1)];
                        e[4] = eay[i * ny + j];
                        e[5] = eay[(i + 1) * ny + j];
                        e[6] = eby[i * ny + j];
                        e[7] = eby[(i + 1) * ny + j];
                        e[8] = ez[i * ny + j];
                        e[9] = ez[(i + 1) * ny + j];
                        e[10] = ez[i * ny + (j + 1)];
                        e[11] = ez[(i + 1) * ny + (j + 1)];

                        for (int h = 0; m_triangle_table[cubeindex][h] != -1; h += 3)
                        {
                            mesh->triangle(e[m_triangle_table[cubeindex][h]], e[m_triangle_table[cubeindex][h + 1]], e[m_triangle_table[cubeindex][h + 2]]);
                        }
                    }
                }
            }

            std::swap(a, b);

            za = zb;
            std::swap(eax, ebx);
            std::swap(eay, eby);
            std::swap(u, v);
        }

        delete[] a;
        delete[] b;
        delete[] u;
        delete[] v;

        delete[] eax;
        delete[] eay;
        delete[] ebx;
        delete[] eby;
        delete[] ez;

        return mesh;
    }

    /*!
    \brief Compute the intersection between a segment and an implicit surface.

    \param a,b End vertices of the segment straddling the surface.
    \param va,vb Field function value at those end vertices.
    \param length Distance between vertices.
    \param s_epsilon Precision.
    \return Point on the implicit surface.
    */
    Vector ImplicitTree::dichotomy(Vector a, Vector b, float va, float vb, float length) const
    {
        int ia = va > 0.0 ? 1 : -1;

        // Get an accurate first guess
        Vector c = (vb * a - va * b) / (vb - va);

        while (length > s_epsilon)
        {
            float vc = value(Point(c));
            int ic = vc > 0.0 ? 1 : -1;
            if (ia + ic == 0)
            {
                b = c;
            }
            else
            {
                ia = ic;
                a = c;
            }
            length *= 0.5;
            c = 0.5 * (a + b);
        }
        return c;
    }

    ImplicitType ImplicitTree::type() const
    {
        return ImplicitType::TREE;
    }

    /*!
    \brief Compute the normal to the surface.

    \sa ImplicitTree::gradient(const Vector&) const

    \param p Point (should be on the surface).
    */
    Vector ImplicitTree::normal(const Vector &p) const
    {
        Vector normal = normalize(gradient(Point(p)));

        return normal;
    }

    int ImplicitTree::m_edge_table[256] = {
        0, 273, 545, 816, 1042, 1283, 1587, 1826, 2082, 2355, 2563, 2834, 3120, 3361, 3601, 3840,
        324, 85, 869, 628, 1366, 1095, 1911, 1638, 2406, 2167, 2887, 2646, 3444, 3173, 3925, 3652,
        644, 917, 165, 436, 1686, 1927, 1207, 1446, 2726, 2999, 2183, 2454, 3764, 4005, 3221, 3460,
        960, 721, 481, 240, 2002, 1731, 1523, 1250, 3042, 2803, 2499, 2258, 4080, 3809, 3537, 3264,
        1096, 1369, 1641, 1912, 90, 331, 635, 874, 3178, 3451, 3659, 3930, 2168, 2409, 2649, 2888,
        1292, 1053, 1837, 1596, 286, 15, 831, 558, 3374, 3135, 3855, 3614, 2364, 2093, 2845, 2572,
        1740, 2013, 1261, 1532, 734, 975, 255, 494, 3822, 4095, 3279, 3550, 2812, 3053, 2269, 2508,
        1928, 1689, 1449, 1208, 922, 651, 443, 170, 4010, 3771, 3467, 3226, 3000, 2729, 2457, 2184,
        2184, 2457, 2729, 3000, 3226, 3467, 3771, 4010, 170, 443, 651, 922, 1208, 1449, 1689, 1928,
        2508, 2269, 3053, 2812, 3550, 3279, 4095, 3822, 494, 255, 975, 734, 1532, 1261, 2013, 1740,
        2572, 2845, 2093, 2364, 3614, 3855, 3135, 3374, 558, 831, 15, 286, 1596, 1837, 1053, 1292,
        2888, 2649, 2409, 2168, 3930, 3659, 3451, 3178, 874, 635, 331, 90, 1912, 1641, 1369, 1096,
        3264, 3537, 3809, 4080, 2258, 2499, 2803, 3042, 1250, 1523, 1731, 2002, 240, 481, 721, 960,
        3460, 3221, 4005, 3764, 2454, 2183, 2999, 2726, 1446, 1207, 1927, 1686, 436, 165, 917, 644,
        3652, 3925, 3173, 3444, 2646, 2887, 2167, 2406, 1638, 1911, 1095, 1366, 628, 869, 85, 324,
        3840, 3601, 3361, 3120, 2834, 2563, 2355, 2082, 1826, 1587, 1283, 1042, 816, 545, 273, 0};

    int ImplicitTree::m_triangle_table[256][16] = {
        {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 5, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {5, 8, 4, 9, 8, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 10, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 10, 1, 8, 10, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {5, 9, 0, 1, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {5, 10, 1, 5, 9, 10, 9, 8, 10, -1, -1, -1, -1, -1, -1, -1},
        {5, 1, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 4, 5, 1, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 1, 11, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 8, 4, 1, 11, 8, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
        {4, 11, 5, 10, 11, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 11, 5, 0, 8, 11, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
        {4, 9, 0, 4, 10, 9, 10, 11, 9, -1, -1, -1, -1, -1, -1, -1},
        {9, 8, 11, 11, 8, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 4, 0, 6, 4, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 5, 9, 8, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 5, 9, 2, 6, 5, 6, 4, 5, -1, -1, -1, -1, -1, -1, -1},
        {8, 2, 6, 4, 10, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {10, 2, 6, 10, 1, 2, 1, 0, 2, -1, -1, -1, -1, -1, -1, -1},
        {9, 0, 5, 8, 2, 6, 1, 4, 10, -1, -1, -1, -1, -1, -1, -1},
        {2, 6, 10, 9, 2, 10, 9, 10, 1, 9, 1, 5, -1, -1, -1, -1},
        {5, 1, 11, 8, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 2, 6, 4, 0, 2, 5, 1, 11, -1, -1, -1, -1, -1, -1, -1},
        {9, 1, 11, 9, 0, 1, 8, 2, 6, -1, -1, -1, -1, -1, -1, -1},
        {1, 11, 9, 1, 9, 6, 1, 6, 4, 6, 9, 2, -1, -1, -1, -1},
        {4, 11, 5, 4, 10, 11, 6, 8, 2, -1, -1, -1, -1, -1, -1, -1},
        {5, 10, 11, 5, 2, 10, 5, 0, 2, 6, 10, 2, -1, -1, -1, -1},
        {2, 6, 8, 9, 0, 10, 9, 10, 11, 10, 0, 4, -1, -1, -1, -1},
        {2, 6, 10, 2, 10, 9, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
        {9, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 7, 2, 0, 8, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 7, 2, 5, 7, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {8, 7, 2, 8, 4, 7, 4, 5, 7, -1, -1, -1, -1, -1, -1, -1},
        {9, 7, 2, 1, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 10, 1, 0, 8, 10, 2, 9, 7, -1, -1, -1, -1, -1, -1, -1},
        {0, 7, 2, 0, 5, 7, 1, 4, 10, -1, -1, -1, -1, -1, -1, -1},
        {1, 5, 7, 1, 7, 8, 1, 8, 10, 2, 8, 7, -1, -1, -1, -1},
        {5, 1, 11, 9, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 0, 8, 5, 1, 11, 2, 9, 7, -1, -1, -1, -1, -1, -1, -1},
        {7, 1, 11, 7, 2, 1, 2, 0, 1, -1, -1, -1, -1, -1, -1, -1},
        {1, 11, 7, 4, 1, 7, 4, 7, 2, 4, 2, 8, -1, -1, -1, -1},
        {11, 4, 10, 11, 5, 4, 9, 7, 2, -1, -1, -1, -1, -1, -1, -1},
        {2, 9, 7, 0, 8, 5, 8, 11, 5, 8, 10, 11, -1, -1, -1, -1},
        {7, 2, 0, 7, 0, 10, 7, 10, 11, 10, 0, 4, -1, -1, -1, -1},
        {7, 2, 8, 7, 8, 11, 11, 8, 10, -1, -1, -1, -1, -1, -1, -1},
        {9, 6, 8, 7, 6, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 4, 0, 9, 7, 4, 7, 6, 4, -1, -1, -1, -1, -1, -1, -1},
        {0, 6, 8, 0, 5, 6, 5, 7, 6, -1, -1, -1, -1, -1, -1, -1},
        {5, 7, 4, 4, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {6, 9, 7, 6, 8, 9, 4, 10, 1, -1, -1, -1, -1, -1, -1, -1},
        {9, 7, 6, 9, 6, 1, 9, 1, 0, 1, 6, 10, -1, -1, -1, -1},
        {1, 4, 10, 0, 5, 8, 5, 6, 8, 5, 7, 6, -1, -1, -1, -1},
        {10, 1, 5, 10, 5, 6, 6, 5, 7, -1, -1, -1, -1, -1, -1, -1},
        {9, 6, 8, 9, 7, 6, 11, 5, 1, -1, -1, -1, -1, -1, -1, -1},
        {11, 5, 1, 9, 7, 0, 7, 4, 0, 7, 6, 4, -1, -1, -1, -1},
        {8, 0, 1, 8, 1, 7, 8, 7, 6, 11, 7, 1, -1, -1, -1, -1},
        {1, 11, 7, 1, 7, 4, 4, 7, 6, -1, -1, -1, -1, -1, -1, -1},
        {9, 7, 8, 8, 7, 6, 11, 5, 4, 11, 4, 10, -1, -1, -1, -1},
        {7, 6, 0, 7, 0, 9, 6, 10, 0, 5, 0, 11, 10, 11, 0, -1},
        {10, 11, 0, 10, 0, 4, 11, 7, 0, 8, 0, 6, 7, 6, 0, -1},
        {10, 11, 7, 6, 10, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {6, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 0, 8, 10, 6, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 5, 9, 10, 6, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {8, 5, 9, 8, 4, 5, 10, 6, 3, -1, -1, -1, -1, -1, -1, -1},
        {6, 1, 4, 3, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {6, 0, 8, 6, 3, 0, 3, 1, 0, -1, -1, -1, -1, -1, -1, -1},
        {1, 6, 3, 1, 4, 6, 0, 5, 9, -1, -1, -1, -1, -1, -1, -1},
        {5, 3, 1, 5, 8, 3, 5, 9, 8, 8, 6, 3, -1, -1, -1, -1},
        {11, 5, 1, 3, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {5, 1, 11, 4, 0, 8, 3, 10, 6, -1, -1, -1, -1, -1, -1, -1},
        {1, 9, 0, 1, 11, 9, 3, 10, 6, -1, -1, -1, -1, -1, -1, -1},
        {3, 10, 6, 1, 11, 4, 11, 8, 4, 11, 9, 8, -1, -1, -1, -1},
        {11, 6, 3, 11, 5, 6, 5, 4, 6, -1, -1, -1, -1, -1, -1, -1},
        {11, 6, 3, 5, 6, 11, 5, 8, 6, 5, 0, 8, -1, -1, -1, -1},
        {0, 4, 6, 0, 6, 11, 0, 11, 9, 3, 11, 6, -1, -1, -1, -1},
        {6, 3, 11, 6, 11, 8, 8, 11, 9, -1, -1, -1, -1, -1, -1, -1},
        {3, 8, 2, 10, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 3, 10, 4, 0, 3, 0, 2, 3, -1, -1, -1, -1, -1, -1, -1},
        {8, 3, 10, 8, 2, 3, 9, 0, 5, -1, -1, -1, -1, -1, -1, -1},
        {9, 2, 3, 9, 3, 4, 9, 4, 5, 10, 4, 3, -1, -1, -1, -1},
        {8, 1, 4, 8, 2, 1, 2, 3, 1, -1, -1, -1, -1, -1, -1, -1},
        {0, 2, 1, 2, 3, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {5, 9, 0, 1, 4, 2, 1, 2, 3, 2, 4, 8, -1, -1, -1, -1},
        {5, 9, 2, 5, 2, 1, 1, 2, 3, -1, -1, -1, -1, -1, -1, -1},
        {3, 8, 2, 3, 10, 8, 1, 11, 5, -1, -1, -1, -1, -1, -1, -1},
        {5, 1, 11, 4, 0, 10, 0, 3, 10, 0, 2, 3, -1, -1, -1, -1},
        {2, 10, 8, 2, 3, 10, 0, 1, 9, 1, 11, 9, -1, -1, -1, -1},
        {11, 9, 4, 11, 4, 1, 9, 2, 4, 10, 4, 3, 2, 3, 4, -1},
        {8, 5, 4, 8, 3, 5, 8, 2, 3, 3, 11, 5, -1, -1, -1, -1},
        {11, 5, 0, 11, 0, 3, 3, 0, 2, -1, -1, -1, -1, -1, -1, -1},
        {2, 3, 4, 2, 4, 8, 3, 11, 4, 0, 4, 9, 11, 9, 4, -1},
        {11, 9, 2, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 9, 7, 6, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 4, 2, 9, 7, 10, 6, 3, -1, -1, -1, -1, -1, -1, -1},
        {7, 0, 5, 7, 2, 0, 6, 3, 10, -1, -1, -1, -1, -1, -1, -1},
        {10, 6, 3, 8, 4, 2, 4, 7, 2, 4, 5, 7, -1, -1, -1, -1},
        {6, 1, 4, 6, 3, 1, 7, 2, 9, -1, -1, -1, -1, -1, -1, -1},
        {9, 7, 2, 0, 8, 3, 0, 3, 1, 3, 8, 6, -1, -1, -1, -1},
        {4, 3, 1, 4, 6, 3, 5, 7, 0, 7, 2, 0, -1, -1, -1, -1},
        {3, 1, 8, 3, 8, 6, 1, 5, 8, 2, 8, 7, 5, 7, 8, -1},
        {9, 7, 2, 11, 5, 1, 6, 3, 10, -1, -1, -1, -1, -1, -1, -1},
        {3, 10, 6, 5, 1, 11, 0, 8, 4, 2, 9, 7, -1, -1, -1, -1},
        {6, 3, 10, 7, 2, 11, 2, 1, 11, 2, 0, 1, -1, -1, -1, -1},
        {4, 2, 8, 4, 7, 2, 4, 1, 7, 11, 7, 1, 10, 6, 3, -1},
        {9, 7, 2, 11, 5, 3, 5, 6, 3, 5, 4, 6, -1, -1, -1, -1},
        {5, 3, 11, 5, 6, 3, 5, 0, 6, 8, 6, 0, 9, 7, 2, -1},
        {2, 0, 11, 2, 11, 7, 0, 4, 11, 3, 11, 6, 4, 6, 11, -1},
        {6, 3, 11, 6, 11, 8, 7, 2, 11, 2, 8, 11, -1, -1, -1, -1},
        {3, 9, 7, 3, 10, 9, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
        {4, 3, 10, 0, 3, 4, 0, 7, 3, 0, 9, 7, -1, -1, -1, -1},
        {0, 10, 8, 0, 7, 10, 0, 5, 7, 7, 3, 10, -1, -1, -1, -1},
        {3, 10, 4, 3, 4, 7, 7, 4, 5, -1, -1, -1, -1, -1, -1, -1},
        {7, 8, 9, 7, 1, 8, 7, 3, 1, 4, 8, 1, -1, -1, -1, -1},
        {9, 7, 3, 9, 3, 0, 0, 3, 1, -1, -1, -1, -1, -1, -1, -1},
        {5, 7, 8, 5, 8, 0, 7, 3, 8, 4, 8, 1, 3, 1, 8, -1},
        {5, 7, 3, 1, 5, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {5, 1, 11, 9, 7, 10, 9, 10, 8, 10, 7, 3, -1, -1, -1, -1},
        {0, 10, 4, 0, 3, 10, 0, 9, 3, 7, 3, 9, 5, 1, 11, -1},
        {10, 8, 7, 10, 7, 3, 8, 0, 7, 11, 7, 1, 0, 1, 7, -1},
        {3, 10, 4, 3, 4, 7, 1, 11, 4, 11, 7, 4, -1, -1, -1, -1},
        {5, 4, 3, 5, 3, 11, 4, 8, 3, 7, 3, 9, 8, 9, 3, -1},
        {11, 5, 0, 11, 0, 3, 9, 7, 0, 7, 3, 0, -1, -1, -1, -1},
        {0, 4, 8, 7, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {11, 7, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {11, 3, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 4, 7, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 0, 5, 7, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {5, 8, 4, 5, 9, 8, 7, 11, 3, -1, -1, -1, -1, -1, -1, -1},
        {1, 4, 10, 11, 3, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {10, 0, 8, 10, 1, 0, 11, 3, 7, -1, -1, -1, -1, -1, -1, -1},
        {0, 5, 9, 1, 4, 10, 7, 11, 3, -1, -1, -1, -1, -1, -1, -1},
        {7, 11, 3, 5, 9, 1, 9, 10, 1, 9, 8, 10, -1, -1, -1, -1},
        {5, 3, 7, 1, 3, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {5, 3, 7, 5, 1, 3, 4, 0, 8, -1, -1, -1, -1, -1, -1, -1},
        {9, 3, 7, 9, 0, 3, 0, 1, 3, -1, -1, -1, -1, -1, -1, -1},
        {7, 9, 8, 7, 8, 1, 7, 1, 3, 4, 1, 8, -1, -1, -1, -1},
        {3, 4, 10, 3, 7, 4, 7, 5, 4, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 10, 0, 10, 7, 0, 7, 5, 7, 10, 3, -1, -1, -1, -1},
        {4, 10, 3, 0, 4, 3, 0, 3, 7, 0, 7, 9, -1, -1, -1, -1},
        {3, 7, 9, 3, 9, 10, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
        {7, 11, 3, 2, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 4, 0, 2, 6, 4, 3, 7, 11, -1, -1, -1, -1, -1, -1, -1},
        {5, 9, 0, 7, 11, 3, 8, 2, 6, -1, -1, -1, -1, -1, -1, -1},
        {11, 3, 7, 5, 9, 6, 5, 6, 4, 6, 9, 2, -1, -1, -1, -1},
        {4, 10, 1, 6, 8, 2, 11, 3, 7, -1, -1, -1, -1, -1, -1, -1},
        {7, 11, 3, 2, 6, 1, 2, 1, 0, 1, 6, 10, -1, -1, -1, -1},
        {0, 5, 9, 2, 6, 8, 1, 4, 10, 7, 11, 3, -1, -1, -1, -1},
        {9, 1, 5, 9, 10, 1, 9, 2, 10, 6, 10, 2, 7, 11, 3, -1},
        {3, 5, 1, 3, 7, 5, 2, 6, 8, -1, -1, -1, -1, -1, -1, -1},
        {5, 1, 7, 7, 1, 3, 4, 0, 2, 4, 2, 6, -1, -1, -1, -1},
        {8, 2, 6, 9, 0, 7, 0, 3, 7, 0, 1, 3, -1, -1, -1, -1},
        {6, 4, 9, 6, 9, 2, 4, 1, 9, 7, 9, 3, 1, 3, 9, -1},
        {8, 2, 6, 4, 10, 7, 4, 7, 5, 7, 10, 3, -1, -1, -1, -1},
        {7, 5, 10, 7, 10, 3, 5, 0, 10, 6, 10, 2, 0, 2, 10, -1},
        {0, 7, 9, 0, 3, 7, 0, 4, 3, 10, 3, 4, 8, 2, 6, -1},
        {3, 7, 9, 3, 9, 10, 2, 6, 9, 6, 10, 9, -1, -1, -1, -1},
        {11, 2, 9, 3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 11, 3, 2, 9, 11, 0, 8, 4, -1, -1, -1, -1, -1, -1, -1},
        {11, 0, 5, 11, 3, 0, 3, 2, 0, -1, -1, -1, -1, -1, -1, -1},
        {8, 4, 5, 8, 5, 3, 8, 3, 2, 3, 5, 11, -1, -1, -1, -1},
        {11, 2, 9, 11, 3, 2, 10, 1, 4, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 1, 1, 8, 10, 2, 9, 11, 2, 11, 3, -1, -1, -1, -1},
        {4, 10, 1, 0, 5, 3, 0, 3, 2, 3, 5, 11, -1, -1, -1, -1},
        {3, 2, 5, 3, 5, 11, 2, 8, 5, 1, 5, 10, 8, 10, 5, -1},
        {5, 2, 9, 5, 1, 2, 1, 3, 2, -1, -1, -1, -1, -1, -1, -1},
        {4, 0, 8, 5, 1, 9, 1, 2, 9, 1, 3, 2, -1, -1, -1, -1},
        {0, 1, 2, 2, 1, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {8, 4, 1, 8, 1, 2, 2, 1, 3, -1, -1, -1, -1, -1, -1, -1},
        {9, 3, 2, 9, 4, 3, 9, 5, 4, 10, 3, 4, -1, -1, -1, -1},
        {8, 10, 5, 8, 5, 0, 10, 3, 5, 9, 5, 2, 3, 2, 5, -1},
        {4, 10, 3, 4, 3, 0, 0, 3, 2, -1, -1, -1, -1, -1, -1, -1},
        {3, 2, 8, 10, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {6, 11, 3, 6, 8, 11, 8, 9, 11, -1, -1, -1, -1, -1, -1, -1},
        {0, 6, 4, 0, 11, 6, 0, 9, 11, 3, 6, 11, -1, -1, -1, -1},
        {11, 3, 6, 5, 11, 6, 5, 6, 8, 5, 8, 0, -1, -1, -1, -1},
        {11, 3, 6, 11, 6, 5, 5, 6, 4, -1, -1, -1, -1, -1, -1, -1},
        {1, 4, 10, 11, 3, 8, 11, 8, 9, 8, 3, 6, -1, -1, -1, -1},
        {1, 0, 6, 1, 6, 10, 0, 9, 6, 3, 6, 11, 9, 11, 6, -1},
        {5, 8, 0, 5, 6, 8, 5, 11, 6, 3, 6, 11, 1, 4, 10, -1},
        {10, 1, 5, 10, 5, 6, 11, 3, 5, 3, 6, 5, -1, -1, -1, -1},
        {5, 1, 3, 5, 3, 8, 5, 8, 9, 8, 3, 6, -1, -1, -1, -1},
        {1, 3, 9, 1, 9, 5, 3, 6, 9, 0, 9, 4, 6, 4, 9, -1},
        {6, 8, 0, 6, 0, 3, 3, 0, 1, -1, -1, -1, -1, -1, -1, -1},
        {6, 4, 1, 3, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {8, 9, 3, 8, 3, 6, 9, 5, 3, 10, 3, 4, 5, 4, 3, -1},
        {0, 9, 5, 10, 3, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {6, 8, 0, 6, 0, 3, 4, 10, 0, 10, 3, 0, -1, -1, -1, -1},
        {6, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {10, 7, 11, 6, 7, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {10, 7, 11, 10, 6, 7, 8, 4, 0, -1, -1, -1, -1, -1, -1, -1},
        {7, 10, 6, 7, 11, 10, 5, 9, 0, -1, -1, -1, -1, -1, -1, -1},
        {11, 6, 7, 11, 10, 6, 9, 8, 5, 8, 4, 5, -1, -1, -1, -1},
        {1, 7, 11, 1, 4, 7, 4, 6, 7, -1, -1, -1, -1, -1, -1, -1},
        {8, 1, 0, 8, 7, 1, 8, 6, 7, 11, 1, 7, -1, -1, -1, -1},
        {9, 0, 5, 7, 11, 4, 7, 4, 6, 4, 11, 1, -1, -1, -1, -1},
        {9, 8, 1, 9, 1, 5, 8, 6, 1, 11, 1, 7, 6, 7, 1, -1},
        {10, 5, 1, 10, 6, 5, 6, 7, 5, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 4, 5, 1, 6, 5, 6, 7, 6, 1, 10, -1, -1, -1, -1},
        {9, 6, 7, 9, 1, 6, 9, 0, 1, 1, 10, 6, -1, -1, -1, -1},
        {6, 7, 1, 6, 1, 10, 7, 9, 1, 4, 1, 8, 9, 8, 1, -1},
        {5, 4, 7, 4, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 6, 0, 6, 5, 5, 6, 7, -1, -1, -1, -1, -1, -1, -1},
        {9, 0, 4, 9, 4, 7, 7, 4, 6, -1, -1, -1, -1, -1, -1, -1},
        {9, 8, 6, 7, 9, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {7, 8, 2, 7, 11, 8, 11, 10, 8, -1, -1, -1, -1, -1, -1, -1},
        {7, 0, 2, 7, 10, 0, 7, 11, 10, 10, 4, 0, -1, -1, -1, -1},
        {0, 5, 9, 8, 2, 11, 8, 11, 10, 11, 2, 7, -1, -1, -1, -1},
        {11, 10, 2, 11, 2, 7, 10, 4, 2, 9, 2, 5, 4, 5, 2, -1},
        {1, 7, 11, 4, 7, 1, 4, 2, 7, 4, 8, 2, -1, -1, -1, -1},
        {7, 11, 1, 7, 1, 2, 2, 1, 0, -1, -1, -1, -1, -1, -1, -1},
        {4, 11, 1, 4, 7, 11, 4, 8, 7, 2, 7, 8, 0, 5, 9, -1},
        {7, 11, 1, 7, 1, 2, 5, 9, 1, 9, 2, 1, -1, -1, -1, -1},
        {1, 7, 5, 1, 8, 7, 1, 10, 8, 2, 7, 8, -1, -1, -1, -1},
        {0, 2, 10, 0, 10, 4, 2, 7, 10, 1, 10, 5, 7, 5, 10, -1},
        {0, 1, 7, 0, 7, 9, 1, 10, 7, 2, 7, 8, 10, 8, 7, -1},
        {9, 2, 7, 1, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {8, 2, 7, 8, 7, 4, 4, 7, 5, -1, -1, -1, -1, -1, -1, -1},
        {0, 2, 7, 5, 0, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {8, 2, 7, 8, 7, 4, 9, 0, 7, 0, 4, 7, -1, -1, -1, -1},
        {9, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 10, 6, 2, 9, 10, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 4, 2, 9, 6, 9, 10, 6, 9, 11, 10, -1, -1, -1, -1},
        {5, 11, 10, 5, 10, 2, 5, 2, 0, 6, 2, 10, -1, -1, -1, -1},
        {4, 5, 2, 4, 2, 8, 5, 11, 2, 6, 2, 10, 11, 10, 2, -1},
        {1, 9, 11, 1, 6, 9, 1, 4, 6, 6, 2, 9, -1, -1, -1, -1},
        {9, 11, 6, 9, 6, 2, 11, 1, 6, 8, 6, 0, 1, 0, 6, -1},
        {4, 6, 11, 4, 11, 1, 6, 2, 11, 5, 11, 0, 2, 0, 11, -1},
        {5, 11, 1, 8, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 10, 6, 9, 10, 2, 9, 1, 10, 9, 5, 1, -1, -1, -1, -1},
        {9, 6, 2, 9, 10, 6, 9, 5, 10, 1, 10, 5, 0, 8, 4, -1},
        {10, 6, 2, 10, 2, 1, 1, 2, 0, -1, -1, -1, -1, -1, -1, -1},
        {10, 6, 2, 10, 2, 1, 8, 4, 2, 4, 1, 2, -1, -1, -1, -1},
        {2, 9, 5, 2, 5, 6, 6, 5, 4, -1, -1, -1, -1, -1, -1, -1},
        {2, 9, 5, 2, 5, 6, 0, 8, 5, 8, 6, 5, -1, -1, -1, -1},
        {2, 0, 4, 6, 2, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 11, 8, 11, 10, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 0, 9, 4, 9, 10, 10, 9, 11, -1, -1, -1, -1, -1, -1, -1},
        {0, 5, 11, 0, 11, 8, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
        {4, 5, 11, 10, 4, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 4, 8, 1, 8, 11, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
        {9, 11, 1, 0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 4, 8, 1, 8, 11, 0, 5, 8, 5, 11, 8, -1, -1, -1, -1},
        {5, 11, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {5, 1, 10, 5, 10, 9, 9, 10, 8, -1, -1, -1, -1, -1, -1, -1},
        {4, 0, 9, 4, 9, 10, 5, 1, 9, 1, 10, 9, -1, -1, -1, -1},
        {0, 1, 10, 8, 0, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 1, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {5, 4, 8, 9, 5, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 9, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 4, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};

} // namespace gm
