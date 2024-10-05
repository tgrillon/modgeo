#include "bezier.h"

#include "image_io.h"

namespace gm
{

/*!
\brief Returns a vector orthogonal to the argument vector.

The returned orthogonal vector is not computed randomly.
First, we find the two coordinates of the argument vector with
maximum absolute value. The orthogonal vector is defined by
swapping those two coordinates and changing one sign, whereas
the third coordinate is set to 0.

The returned orthogonal vector lies in the plane orthogonal
to the first vector.
*/
Vector orthogonal(const Vector& v) 
{
    float c[3]= { v.x, v.y, v.z };
    float a[3]= { abs(v.x), abs(v.y), abs(v.z) };
    int i = 0;
    int j = 1;
    if (a[0] > a[1])
    {
        if (a[2] > a[1])
        {
            j = 2;
        }
    }
    else
    {
        i = 1;
        j = 2;
        if (a[0] > a[2])
        {
            j = 0;
        }
    }
    float b[3]= { 0., 0., 0. };
    b[i] = c[j];
    b[j] = -c[i];
    return Vector(b[0], b[1], b[2]);
}

std::vector<Point> curve_points(int n, const std::function<Point(double)>& f)
{
    std::vector<Point> points;
    double step= 1.0 / (n - 1);

    for (int i= 0; i < n; ++i)
    {
        double t= i * step;
        points.emplace_back(f(t));
    }

    return points;
}

std::vector<std::vector<Point>> surface_points(int n, const std::function<Point(double, double)>& f)
{
    std::vector<std::vector<Point>> points; 
    double step= 1.0 / (n - 1);

    for (int i= 0; i < n; ++i)
    {
        std::vector<Point> row; 
        double u= i * step;
        for (int j= 0; j < n; ++j)
        {
            double v= j * step;  
            row.emplace_back(f(u, v));
        }

        points.push_back(row);
    }
    return points;
}

double bernstein(double t, int k, int n)
{
    return binomal_coeffs[n][k]  * pow(t, k) * pow(1 - t, n - k);
}

Point first_derivative(const Curve& c, double t, double e)
{
    return Point((c.point_curve(t + e) - c.point_curve(t - e))) / (2 * e);
}

Point second_derivative(const Curve& c, double t, double e)
{
    return (c.point_curve(t + e) - 2 * c.point_curve(t) + c.point_curve(t - e)) / (e * e);
}

/**************** CURVE ****************/

Vector Curve::tangente(double t) const
{
    return normalize(Vector(first_derivative(*this, t, 0.0001)));
}

Vector Curve::binormal(double t) const
{
    return normalize(cross(tangente(t), normal(t))); 
}

/**************** SPLINE ****************/

Spline::Spline() : Curve()
{
}

Spline::Spline(const std::vector<Point> &points) : Curve(), m_control_points(points)
{
}

Spline Spline::create(const std::vector<Point> &points)
{
    return Spline(points); 
}

Point Spline::point_curve(double t) const
{
    Point p; 
    for (int c= 0; c < m_control_points.size(); ++c)
    {
        p= p + m_control_points[c] * bernstein(t, c, m_control_points.size()-1); 
    }

    return p; 
}

Revolution::Revolution(const std::vector<Point>& points) : Spline(points)
{
}

Revolution Revolution::create(const std::vector<Point>& points)
{
  return Revolution(points);
}

void Revolution::radial_fun(const std::function<double(double, double)> &f)
{
    m_radial_fun= f;
}

Mesh Revolution::polygonize(int n) const
{
    assert(n > 2);
    assert(point_count() > 1);

    Mesh mesh(GL_TRIANGLES);
    double step= 1.0 / (n - 1);

    Vector ortho= tangente(step);
    m_ortho_vec= orthogonal(ortho);

    for (int i= 0; i < n; ++i)
    { 
        double u= step * i; 
        for (int j= 0; j <= n; ++j)
        {
            double v= step * j; 
            Point p= point(u, v);
            mesh.vertex(p);

            if (i > 0 && j > 0)
            {
                int prev_i= i - 1;
                int prev_j= j - 1;

                mesh.triangle(prev_i * n + prev_j, i * n + j, i * n + prev_j);
                mesh.triangle(prev_i * n + prev_j, prev_i * n + j, i * n + j);
            }
        }
    }

    return mesh;
}

Point Revolution::point(double u, double theta) const
{
    return point_curve(u) + m_radial_fun(u, theta) * (cos(2 *  M_PI * theta) * normal(u) + sin(2 *  M_PI * theta) * binormal(u)); 
}
 
int Spline::point_count() const
{
  return m_control_points.size();
}

Vector Spline::normal(double t) const
{
    Vector tng= tangente(t);

    return normalize(cross(tng, m_ortho_vec));
}

/**************** Bezier ****************/

Bezier::Bezier() : m_control_points({})
{
}

Bezier::Bezier(const std::vector<std::vector<Point>> &points) : m_control_points(points)
{
}

Bezier Bezier::create(const std::vector<std::vector<Point>> &points)
{
  return Bezier(points);
}

Mesh Bezier::polygonize(int n) const
{
    assert(n > 2);
    assert(point_count() > 4);

    Mesh mesh(GL_TRIANGLES); 
    double step= 1.0 / (n - 1);

    for (int i= 0; i < n; ++i)
    { 
        double u= step * i; 
        for (int j= 0; j < n; ++j)
        {
            double v= step * j; 
            Point p= point(u, v);
            mesh.vertex(p);

            if (i > 0 && j > 0)
            {
                int prev_i= i - 1;
                int prev_j= j - 1;

                mesh.triangle(prev_i * n + prev_j, i * n + prev_j, i * n + j);
                mesh.triangle(prev_i * n + prev_j, i * n + j, prev_i * n + j);
            }
        }
    }

    return mesh;
}

Point Bezier::point(double u, double v) const
{
    Point p; 
    for (int r= 0; r < height(); ++r)
    {
        for (int c= 0; c < width(); ++c)
        {
            p= p + bernstein(u, c, width()-1) * bernstein(v, r, height()-1) * m_control_points[r][c]; 
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