#include "Bezier.h"

#include "image_io.h"

namespace mg
{

std::vector<Point> genetrate_points(int resolution, std::function<Point(double)> fun)
{
    double step= 1.0 / (resolution - 1);

    std::vector<Point> points;
    for (int i= 0; i < resolution; ++i)
    {
        double t= i * step;
        points.emplace_back(fun(t));
    }

    return points;
}

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

Point first_derivative(const Curve& c, double t, double e)
{
    return Point((c.point(t + e) - c.point(t - e))) / (2 * e);
}

Point second_derivative(const Curve& c, double t, double e)
{
    return (c.point(t + e) - 2 * c.point(t) + c.point(t - e)) / (e * e);
}

Bezier::Bezier() : m_control_points()
{
}

Bezier::Bezier(const Grid &grid) : m_control_points(grid)
{
}

Mesh Bezier::poligonize(int n) const
{
    assert(n > 3);

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
        }
    }

    for (int i= 0; i < n - 1; ++i)
    { 
        for (int j= 0; j < n - 1; ++j)
        {
            int next_i= i + 1;
            int next_j= j + 1;
            mesh.triangle(i * n + j, next_i * n + j, next_i * n + next_j);
            mesh.triangle(i * n + j, next_i * n + next_j, i * n + next_j);
        }
    }

    return mesh;
}

Bezier Bezier::create(const Grid& grid)
{
    return Bezier(grid); 
}

// Bezier> Bezier::create(int n, int m, double dt)
// {
//   std::random_device dev;
//   std::mt19937 rng(dev());
//   std::uniform_real_distribution<float> dist(-0, 10); 

//   Bezier bz; 
//   bz.m_patch_width= n; 
//   bz.m_patch_height= m;
//   for (int i= 0; i < n; ++i)
//     for (int j= 0; j < m; ++j)
//     {
//       bz.m_control_points.emplace_back(i, sin(dt + i) / (dt + i), j);
//     }

//   return std::make_shared<Bezier>(bz); 
// }

Point Bezier::point(double u, double v) const
{
    Point p; 
    for (int r= 0; r < height(); ++r)
    {
        for (int c= 0; c < width(); ++c)
        {
            p= p + bernstein(u, c, width()-1) * bernstein(v, r, height()-1) * m_control_points.at(r * width() + c); 
        }
    }

    return p; 
}

double Bezier::bernstein(double u, int i, int m) const
{
    return binomal_coeffs[m][i] * pow(u, i) * pow(1 - u, m - i);
}

Grid Grid::create(unsigned int w, unsigned int h)
{
    Grid g; 
    g.m_width= w;
    g.m_height= h;

    for (int z= 0; z < h; ++z)
        for (int x= 0; x < w; ++x)
            g.m_points.emplace_back(x, 0, z);

    return g;
}

Grid Grid::load(const std::string &height_map, float scale)
{
    Image hm= read_image(height_map.c_str());
    Grid g; 

    g.m_width= 30;//hm.width(); 
    g.m_height= 30;//hm.height(); 

    int start_x= 30;
    int start_z= 30;

    for (int z= start_z; z < g.m_height + start_z/*hm.height()*/; ++z)
        for (int x= start_x; x < g.m_width + start_x/*hm.width()*/; ++x)
            g.m_points.emplace_back(static_cast<float>(x), scale * hm(x, z).r, static_cast<float>(z));

    return g;
}

void Grid::operator()(unsigned int x, unsigned int y, const Point& value)
{
    assert(x < m_width && y < m_height);

    m_points[y * m_width + x]= value;
}

std::vector<Grid> load(const std::string &height_map, unsigned int max_grid_width, unsigned int max_grid_height, float scale)
{
    Image hm= read_image(height_map.c_str());
    std::vector<Grid> g; 

    unsigned int num_of_vPatch= hm.height() / max_grid_height;
    unsigned int num_of_hPatch= hm.width() / max_grid_width;

    g.resize(num_of_vPatch * num_of_hPatch);

    for (const auto& [index, patch] : utils::enumerate(g))
    {
        for (int z= num_of_hPatch * index; z < num_of_hPatch * index + max_grid_height; ++z)
        {
            for (int x= num_of_vPatch * index; x < num_of_vPatch * index + max_grid_width; ++x)
            {
                patch.m_points.emplace_back(static_cast<float>(x), scale * hm(x, z).r, static_cast<float>(z));
            }
        }
    }

    return g;
}

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

Mesh Spline::poligonize(int n, GLenum type)
{
    assert(n > 2);
    assert(m_control_points.size() > 1);

    Mesh mesh(type);
    double step= 1.0 / (n - 1);

    Vector ortho= tangente(step);
    m_ortho_vec= orthogonal(ortho);

    switch(type)
    {
        case GL_LINE_STRIP:
            for (int i= 0; i < n; ++i)
            {
                double u= step * i;
                for (int j= 0; j < n; ++j)
                {
                    double v= step * j;
                    mesh.vertex(point(u, v));
                }
            }
            break;
        case GL_TRIANGLES:
            for (int i= 0; i < n; ++i)
            { 
                double u= step * i; 
                for (int j= 0; j < n; ++j)
                {
                    double v= step * j; 
                    Point p= point(u, v);
                    mesh.vertex(p);
                }
            }

            for (int i= 0; i < n-1; ++i)
            { 
                for (int j= 0; j < n-1; ++j)
                {
                    int next_i= i + 1;
                    int next_j= j + 1;
                    mesh.triangle(i * n + j, next_i * n + next_j, next_i * n + j);
                    mesh.triangle(i * n + j, i * n + next_j, next_i * n + next_j);
                }
            }
            break;
    }

    return mesh;
}

Point Spline::point(double t) const
{
    Point p; 
    for (int c= 0; c < m_control_points.size(); ++c)
    {
        p= p + m_control_points[c] * bernstein(t, c, m_control_points.size()-1); 
    }

    return p; 
}

double Spline::bernstein(double t, int k, int n) const
{
    return binomal_coeffs[n][k]  * pow(t, k) * pow(1 - t, n - k);
}

Point Spline::point(double u, double theta) const
{
    return point(u) + (sin(10 * 2 * M_PI * theta) * 10 * u) * (cos(2 *  M_PI * theta) * normal(u) + sin(2 *  M_PI * theta) * binormal(u));
}

Vector Spline::normal(double t) const
{
    Vector tng= tangente(t);

    return normalize(cross(tng, m_ortho_vec));
}
} // namespace mg
