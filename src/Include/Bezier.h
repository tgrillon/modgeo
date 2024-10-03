#pragma once

#include <vector>
#include <cmath>
#include <random> 
#include <memory> 
#include <functional> 

#include "mesh.h"
#include "vec.h"

#include "utils.h"

// Modélisation Géométrique
namespace mg 
{

const std::vector<std::vector<int>> binomal_coeffs = {
  {1},
  {1, 1},
  {1, 2, 1},
  {1, 3, 3, 1},
  {1, 4, 6, 4, 1},
  {1, 5, 10, 10, 5, 1},
  {1, 6, 15, 20, 15, 6, 1},
  {1, 7, 21, 35, 35, 21, 7, 1},
  {1, 8, 28, 56, 70, 56, 28, 8, 1},
  {1, 9, 36, 84, 126, 126, 84, 36, 9, 1},
  {1, 10, 45, 120, 210, 252, 210, 120, 45, 10, 1},
  {1, 11, 55, 165, 330, 462, 462, 330, 165, 55, 11, 1},
  {1, 12, 66, 220, 495, 792, 924, 792, 495, 220, 66, 12, 1},
  {1, 13, 78, 286, 715, 1287, 1716, 1716, 1287, 715, 286, 78, 13, 1},
  {1, 14, 91, 364, 1001, 2002, 3003, 3432, 3003, 2002, 1001, 364, 91, 14, 1},
  {1, 15, 105, 455, 1365, 3003, 5005, 6435, 6435, 5005, 3003, 1365, 455, 105, 15, 1},
  {1, 16, 120, 560, 1820, 4368, 8008, 11440, 12870, 11440, 8008, 4368, 1820, 560, 120, 16, 1},
  {1, 17, 136, 680, 2380, 6188, 12376, 19448, 24310, 24310, 19448, 12376, 6188, 2380, 680, 136, 17, 1},
  {1, 18, 153, 816, 3060, 8568, 18564, 31824, 43758, 48620, 43758, 31824, 18564, 8568, 3060, 816, 153, 18, 1},
  {1, 19, 171, 969, 3876, 11628, 27132, 50388, 75582, 92378, 92378, 75582, 50388, 27132, 11628, 3876, 969, 171, 19, 1},
  {1, 20, 190, 1140, 4845, 15504, 38760, 77520, 125970, 167960, 184756, 167960, 125970, 77520, 38760, 15504, 4845, 1140, 190, 20, 1},
  {1, 21, 210, 1330, 5985, 20349, 54264, 116280, 203490, 293930, 352716, 352716, 293930, 203490, 116280, 54264, 20349, 5985, 1330, 210, 21, 1},
  {1, 22, 231, 1540, 7315, 26334, 74613, 170544, 319770, 497420, 646646, 705432, 646646, 497420, 319770, 170544, 74613, 26334, 7315, 1540, 231, 22, 1},
  {1, 23, 253, 1771, 8855, 33649, 100947, 245157, 490314, 817190, 1144066, 1352078, 1352078, 1144066, 817190, 490314, 245157, 100947, 33649, 8855, 1771, 253, 23, 1},
  {1, 24, 276, 2024, 10626, 42504, 134596, 346104, 735471, 1307504, 1961256, 2496144, 2704156, 2496144, 1961256, 1307504, 735471, 346104, 134596, 42504, 10626, 2024, 276, 24, 1},
  {1, 25, 300, 2300, 12650, 53130, 177100, 480700, 1081575, 2042975, 3268760, 4457400, 5200300, 5200300, 4457400, 3268760, 2042975, 1081575, 480700, 177100, 53130, 12650, 2300, 300, 25, 1},
  {1, 26, 325, 2600, 14950, 65780, 230230, 657800, 1562275, 3124550, 5311735, 7726160, 9657700, 10400600, 9657700, 7726160, 5311735, 3124550, 1562275, 657800, 230230, 65780, 14950, 2600, 325, 26, 1},
  {1, 27, 351, 2925, 17550, 80730, 296010, 888030, 2220075, 4686825, 8436285, 13037895, 17383860, 20058300, 20058300, 17383860, 13037895, 8436285, 4686825, 2220075, 888030, 296010, 80730, 17550, 2925, 351, 27, 1},
  {1, 28, 378, 3276, 20475, 98280, 376740, 1184040, 3108105, 6906900, 13123110, 21474180, 30421755, 37442160, 40116600, 37442160, 30421755, 21474180, 13123110, 6906900, 3108105, 1184040, 376740, 98280, 20475, 3276, 378, 28, 1},
  {1, 29, 406, 3654, 23751, 118755, 475020, 1560780, 4292145, 10015005, 20030010, 34597290, 51895935, 67863915, 77558760, 77558760, 67863915, 51895935, 34597290, 20030010, 10015005, 4292145, 1560780, 475020, 118755, 23751, 3654, 406, 29, 1},
  {1, 30, 435, 4060, 27405, 142506, 593775, 2035800, 5852925, 14307150, 30045015, 54627300, 86493225, 119759850, 145422675, 155117520, 145422675, 119759850, 86493225, 54627300, 30045015, 14307150, 5852925, 2035800, 593775, 142506, 27405, 4060, 435, 30, 1},
  {1, 31, 465, 4495, 31465, 169911, 736281, 2629575, 7888725, 20160075, 44352165, 84672315, 141120525, 206253075, 265182525, 300540195, 300540195, 265182525, 206253075, 141120525, 84672315, 44352165, 20160075, 7888725, 2629575, 736281, 169911, 31465, 4495, 465, 31, 1}
}; // 32

std::vector<Point> genetrate_points(int resolution, std::function<Point(double)> fun);

class Curve
{
public:
  Curve()=default;

  inline virtual Vector tangente(double t) const { return normalize(Vector(first_derivative(*this, t, 0.0001))); } 
  virtual Vector normal(double t) const=0;
  inline virtual Vector binormal(double t) const { return normalize(cross(tangente(t), normal(t))); } 

  virtual Point point(double t) const=0;

  friend Point first_derivative(const Curve& c, double t, double e);
  friend Point second_derivative(const Curve& c, double t, double e);
};

class Spline : public Curve
{
public:
  Spline();
  Spline(const std::vector<Point>& points);

  virtual Vector normal(double t) const;

  static Spline create(const std::vector<Point>& points);

  Mesh poligonize(int resolution, GLenum type= GL_TRIANGLES);

  virtual Point point(double t) const;

  double bernstein(double u, int i, int m) const;
  
  Point point(double u, double theta) const;

protected:
  Vector m_ortho_vec;
  std::vector<Point> m_control_points;
};

class Grid
{
public: 
  Grid()=default; 
  ~Grid()=default; 

  //! Create an n*m flat grid. 
  static Grid create(unsigned int n, unsigned int m);
  //! Load the grid corresponding to the provided height map. 
  static Grid load(const std::string& height_map, float scale= 100.0);

  void operator()(unsigned int x, unsigned int y, const Point& value); 
  const Point& operator[](size_t i) { return m_points[i]; } 
  const Point& at(size_t i) const { return m_points.at(i); }  
  void operator=(const std::vector<Point>& points) { m_points= points; } 

  inline size_t width() const { return m_width; }
  inline size_t height() const { return m_height; }

  friend std::vector<Grid> load(const std::string& height_map, unsigned int max_grid_width, unsigned int max_grid_height, float scale);

private: 
  mutable std::vector<Point> m_points{}; 

  unsigned int m_width{0}, m_height{0};
};

// std::vector<Grid>* load(const std::string& height_map, unsigned int max_size_grid= 10, float scale= 100.0);

class Bezier
{
public:
  Bezier();
  Bezier(const Grid& grid);

  Mesh poligonize(int resolution) const; 
  
  static Bezier create(const Grid& grid);

  Grid& control_point() { return m_control_points; }

  inline size_t width() const { return m_control_points.width(); }
  inline size_t height() const { return m_control_points.height(); }

private: 

  Point point(double u, double v) const;

  double bernstein(double u, int i, int m) const;

private:
  Grid m_control_points; 
};
} // namespace mg