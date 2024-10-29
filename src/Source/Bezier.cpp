#include "Bezier.h"

#include <fstream>

namespace gm
{
    /******************* OBJECT *******************/

    Object::Object(const std::vector<Ref<Bezier>> &patches) : m_patches(patches)
    {
    }

    void Object::load_pacthes(const std::string &filename)
    {
        std::ifstream file(filename);

        if (!file.is_open())
        {
            utils::error("Load_patch: Can't open ", filename);
            exit(1);
        }

        float x, y, z;
        std::vector<int> indices;
        std::vector<Point> positions;

        // Read patch indices
        int nb_patches;
        file >> nb_patches;
        indices.resize(nb_patches * 16);
        for (int i = 0; i < indices.size(); i += 16)
        {
            file >> indices[i] >> indices[i + 1] >> indices[i + 2] >> indices[i + 3] >> indices[i + 4] >> indices[i + 5] >> indices[i + 6] >> indices[i + 7] >> indices[i + 8] >> indices[i + 9] >> indices[i + 10] >> indices[i + 11] >> indices[i + 12] >> indices[i + 13] >> indices[i + 14] >> indices[i + 15];
        }

        // Read vertices
        int nb_verticies;
        file >> nb_verticies;
        positions.resize(nb_verticies);
        for (int i = 0; i < nb_verticies; i++)
        {
            file >> x >> y >> z;

            positions[i] = {x, y, z};
        }

        m_patches.resize(indices.size() / 16);

        // Generate control points
        for (int i = 0; i < indices.size(); i += 16)
        {
            std::vector<std::vector<Point>> points;
            points.reserve(4);

            for (int j = i; j < i + 16; j += 4)
            {
                std::vector<Point> pts;
                pts.reserve(4);
                pts.emplace_back(positions[indices[j]]);
                pts.emplace_back(positions[indices[j + 1]]);
                pts.emplace_back(positions[indices[j + 2]]);
                pts.emplace_back(positions[indices[j + 3]]);
                points.emplace_back(pts);
            }

            m_patches[i % 15] = Bezier::create(points);
        }
    }

    Ref<Mesh> Object::polygonize(int n) const
    {
        Ref<Mesh> mesh = create_ref<Mesh>(GL_TRIANGLES);
        double step = 1.0 / (n - 1);

        for (int k = 0; k < m_patches.size(); ++k)
        {
            for (int i = 0; i < n; ++i)
            {
                double u = step * i;
                for (int j = 0; j < n; ++j)
                {
                    double v = step * j;
                    Point p = m_patches[k]->point(u, v);
                    mesh->vertex(p);

                    if (i > 0 && j > 0)
                    {
                        int prev_i = i - 1;
                        int prev_j = j - 1;
                        int curr_i = i;
                        int curr_j = j;

                        int knn = k * n * n;
                        mesh->triangle(knn + prev_i * n + prev_j, knn + curr_i * n + prev_j, knn + curr_i * n + curr_j);
                        mesh->triangle(knn + prev_i * n + prev_j, knn + curr_i * n + curr_j, knn + prev_i * n + curr_j);
                    }
                }
            }
        }

        return mesh;
    }

    std::vector<Point> curve_points(int n, const std::function<Point(double)> &f)
    {
        std::vector<Point> points;
        double step = 1.0 / (n - 1);

        for (int i = 0; i < n; ++i)
        {
            double t = i * step;
            points.emplace_back(f(t));
        }

        return points;
    }

    std::vector<std::vector<Point>> surface_points(int n, const std::function<Point(double, double)> &f)
    {
        std::vector<std::vector<Point>> points;
        double step = 1.0 / (n - 1);

        for (int i = 0; i < n; ++i)
        {
            std::vector<Point> row;
            double u = i * step;
            for (int j = 0; j < n; ++j)
            {
                double v = j * step;
                row.emplace_back(f(u, v));
            }

            points.push_back(row);
        }
        return points;
    }

    double bernstein(double t, int k, int n)
    {
        return binomal_coeffs[n][k] * pow(t, k) * pow(1 - t, n - k);
    }

    Vector Curve::first_derivative(double t, double e) const
    {
        return (point_curve(t + e) - point_curve(t - e)) / (2 * e);
    }

    Vector Curve::second_derivative(double t, double e) const
    {
        return Vector(point_curve(t + e) - 2 * point_curve(t) + point_curve(t - e)) / (e * e);
    }

    /**************** CURVE ****************/

    Vector Curve::tangent(double t) const
    {
        Vector tangent = normalize(Vector(first_derivative(t)));
        assert(length2(tangent) > 0);
        return tangent;
    }

    Vector Curve::binormal(double t) const
    {
        Vector tng = tangent(t);
        Vector nrm = normal(t);
        return normalize(cross(tng, nrm));
    }

    /**************** SPLINE ****************/

    Spline::Spline() : Curve()
    {
    }

    Spline::Spline(const std::vector<Point> &points, Type type) : Curve(), m_control_points(points), m_type(type)
    {
    }

    Ref<Spline> Spline::create(const std::vector<Point> &points, Type type)
    {
        return create_ref<Spline>(points, type);
    }

    double Spline::get_t(double t, unsigned int ip0, unsigned int ip1) const
    {
        Vector d(m_control_points[ip0], m_control_points[ip1]);
        double a = length2(d);
        double b = pow(a, m_alpha * .5);
        return b + t;
    }

    Point Spline::point_curve(double t) const
    {
        Point p;
        if (m_type == Type::BEZIER)
        {
            for (int k = 0; k < point_count(); ++k)
            {
                p = p + bernstein(t, k, point_count() - 1) * m_control_points[k];
            }
        }
        else if (m_type == Type::CATMULL_ROM)
        {
            // unsigned int pi= static_cast<int>(t) * 3;

            // unsigned int p0= pi;
            // unsigned int p1= p0+1;
            // unsigned int p2= p1+1;
            // unsigned int p3= p2+1;

            // double t0= 0.0;
            // double t1= get_t(t, p0, p1);
            // double t2= get_t(t, p1, p2);
            // double t3= get_t(t, p2, p3);

            // t= lerp(t1, t2, t);

            // Point A1= (t1-t) / (t1-t0) * m_control_points[p0] + (t-t0) / (t1-t0) * m_control_points[p1];
            // Point A2= (t2-t) / (t2-t1) * m_control_points[p1] + (t-t1) / (t2-t1) * m_control_points[p2];
            // Point A3= (t3-t) / (t3-t2) * m_control_points[p2] + (t-t2) / (t3-t2) * m_control_points[p3];
            // Point B1= (t2-t) / (t2-t0) * A1 + (t-t0) / (t2-t0) * A2;
            // Point B2= (t3-t) / (t3-t1) * A2 + (t-t1) / (t3-t1) * A3;
            // p= (t2-t) / (t2-t1) * B1 + (t-t1) / (t2-t1) * B2;

            int p0 = static_cast<int>(t);
            int p1 = p0 + 1;
            int p2 = p1 + 1;
            int p3 = p2 + 1;

            t = t - static_cast<int>(t);

            double tt = t * t;
            double ttt = tt * t;

            double q1 = -ttt + 2.0 * tt - t;
            double q2 = 3.0 * ttt - 5.0 * tt + 2.0;
            double q3 = -3.0 * ttt + 4.0 * tt + t;
            double q4 = ttt - tt;

            p = 0.5 * (m_control_points[p0] * q1 + m_control_points[p1] * q2 + m_control_points[p2] * q3 + m_control_points[p3] * q4);
        }

        return p;
    }

    Revolution::Revolution(const std::vector<Point> &points, Type type) : Spline(points, type)
    {
    }

    Ref<Revolution> Revolution::create(const std::vector<Point> &points, Type type)
    {
        return create_ref<Revolution>(points, type);
    }

    void Revolution::radial_fun(const std::function<double(double, double)> &f)
    {
        m_radial_fun = f;
    }

    Ref<Mesh> Revolution::polygonize(int n) const
    {
        assert(n > 2);
        assert(point_count() > 1);

        Ref<Mesh> mesh = create_ref<Mesh>(GL_TRIANGLES);
        int nb_spline = 1;
        if (m_type == Type::CATMULL_ROM)
        {
            nb_spline = (point_count());
        }

        double step = 1.0 / (n - 1);

        for (int i = 0; i < n; ++i)
        {
            double u = step * i;
            Point pc = point_curve(u);
            for (int j = 0; j <= n; ++j)
            {
                if (j < n)
                {
                    double v = 2 * M_PI * step * j;
                    Point p = pc + point(u, v);
                    mesh->vertex(p);
                }

                if (i > 0 && j > 0)
                {
                    int prev_i = i - 1;
                    int prev_j = j - 1;
                    int curr_j = j % n;
                    mesh->triangle(prev_i * n + prev_j, i * n + prev_j, i * n + curr_j);
                    mesh->triangle(prev_i * n + prev_j, i * n + curr_j, prev_i * n + curr_j);
                }
            }
        }

        return mesh;
    }

    Point Revolution::point(double u, double theta) const
    {
        return Point(m_radial_fun(u, theta) * (cos(theta) * normal(u) + sin(theta) * binormal(u)));
    }

    int Spline::point_count() const
    {
        return m_control_points.size();
    }

    Vector Spline::first_derivative(double t, double e) const
    {
        Point p;
        int n = point_count() - 1;
        for (int k = 0; k < n; ++k)
        {
            p = p + n * bernstein(t, k, n - 1) * (m_control_points[k + 1] - m_control_points[k]);
        }

        return Vector(p);
    }

    Vector Spline::second_derivative(double t, double e) const
    {
        Point p;
        int n = point_count() - 1;
        for (int k = 0; k < n - 2; ++k)
        {
            p = p + n * (n - 1) * bernstein(t, k, n - 2) * (m_control_points[k + 2] - 2 * m_control_points[k + 1] + m_control_points[k]);
        }

        return Vector(p);
    }

    Vector Curve::normal(double t) const
    {
        Vector tng = tangent(t);

        Vector arbitrary(0, 1, 0);
        if (fabs(dot(tng, arbitrary)) > 0.99)
        {
            arbitrary = Vector(0, 0, 1);
        }

        return normalize(cross(tng, arbitrary));
        // return normalize(Vector(second_derivative(t)));
    }

    /**************** Bezier ****************/

    Bezier::Bezier() : m_control_points({})
    {
    }

    Bezier::Bezier(const std::vector<std::vector<Point>> &points) : m_control_points(points)
    {
    }

    Ref<Bezier> Bezier::create(const std::vector<std::vector<Point>> &points)
    {
        return create_ref<Bezier>(points);
    }

    Ref<Mesh> Bezier::polygonize(int n) const
    {
        assert(n > 2);
        assert(point_count() > 4);

        Ref<Mesh> mesh = create_ref<Mesh>(GL_TRIANGLES);
        double step = 1.0 / (n - 1);

        for (int i = 0; i < n; ++i)
        {
            double u = step * i;
            for (int j = 0; j < n; ++j)
            {
                double v = step * j;
                Point p = point(u, v);
                mesh->vertex(p);

                if (i > 0 && j > 0)
                {
                    int prev_i = i - 1;
                    int prev_j = j - 1;

                    mesh->triangle(prev_i * n + prev_j, i * n + prev_j, i * n + j);
                    mesh->triangle(prev_i * n + prev_j, i * n + j, prev_i * n + j);
                }
            }
        }

        return mesh;
    }

    Point Bezier::point(double u, double v) const
    {
        Point p;
        for (int r = 0; r < height(); ++r)
        {
            for (int c = 0; c < width(); ++c)
            {
                p = p + bernstein(u, c, width() - 1) * bernstein(v, r, height() - 1) * m_control_points[r][c];
            }
        }

        return p;
    }

    int Bezier::height() const
    {
        return m_control_points.size();
    }

    int Bezier::width() const
    {
        return m_control_points.empty() ? 0 : m_control_points[0].size();
    }

    int Bezier::point_count() const
    {
        return height() * width();
    }

} // namespace mg
