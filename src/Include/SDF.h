#pragma once

#include "pch.h"

#include "Box.h"
#include "Utils.h"

/*
SDF point -> line AB

h= min(1, max(0, dot(ap, ab)/dot(ab, ab)))
d= length(P - A - h * ab)

*/

namespace gm
{
    struct Ray
    {
        Point origin{0, 0, 0};
        Vector direction{0, 0, 0};

        Point point(float t) const;
    };

    enum class IntersectMethod
    {
        RAY_MARCHING = 0,
        SPHERE_TRACING,
        NB_ELT
    };

    enum class SDFType
    {
        TREE = 0,
        PRIMITIVE_SPHERE,
        PRIMITIVE_BOX,
        PRIMITIVE_TORUS,
        PRIMITIVE_CYLINDER,
        PRIMITIVE_CONE,
        PRIMITIVE_PLANE,
        PRIMITIVE_CAPSULE,
        PRIMITIVE_ELLIPSOID,
        PRIMITIVE_OCTAHEDRON,
        PRIMITIVE_PYRAMID,
        UNARY_OPERATOR_HULL,
        UNARY_OPERATOR_ROUNDING,
        UNARY_OPERATOR_ELONGATION,
        UNARY_OPERATOR_REPETITION,
        BINARY_OPERATOR_UNION,
        BINARY_OPERATOR_SMOOTH_UNION,
        BINARY_OPERATOR_INTERSECTION,
        BINARY_OPERATOR_XOR,
        BINARY_OPERATOR_SMOOTH_INTERSECTION,
        BINARY_OPERATOR_SUBSTRACTION,
        BINARY_OPERATOR_SMOOTH_SUBSTRACTION,
        TRANSFORM_TRANSLATION,
        TRANSFORM_ROTATION,
        TRANSFORM_ROTATION_X,
        TRANSFORM_ROTATION_Y,
        TRANSFORM_ROTATION_Z,
        TRANSFORM_SCALE,
        NB_ELT
    };

    /************************** SDF Node ******************************/

    class SDFNode
    {
    public:
        SDFNode(float lambda = 1.0, IntersectMethod method = IntersectMethod::RAY_MARCHING);
        virtual ~SDFNode() = default;

        virtual float value(const Point &p) const;
        virtual bool inside(const Point &p) const;
        virtual Vector gradient(const Point &p) const;
        virtual bool intersect(const Ray &ray, float eps) const;

        void intersect_method(IntersectMethod method);

        virtual Ref<SDFNode> left();
        virtual Ref<SDFNode> right();

        std::pair<Ref<SDFNode>, Ref<SDFNode>> children();

        int value_call_count() const;
        void reset_value_call_count();

        virtual SDFType type() const = 0;

    private:
        bool intersect_ray_marching(const Ray &ray, float eps = 1e-3) const;
        bool intersect_sphere_tracing(const Ray &ray, float t) const;

    protected:
        static const float s_epsilon; //!< Epsilon value for partial derivatives
        static const int s_limit;     //!< Epsilon value for intersection limit
        static int s_value_call_count;

    protected:
        float m_lambda{1.0};

        IntersectMethod m_intersect_method;
    };

    /************************** SDF Unary Operator ******************************/

    class SDFUnaryOperator : public SDFNode
    {
    public:
        SDFUnaryOperator(const Ref<SDFNode> &n, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        virtual ~SDFUnaryOperator() = default;

        virtual float value(const Point &p) const = 0;

        Ref<SDFNode> left() override;
        Ref<SDFNode> right() override;

        virtual SDFType type() const = 0;

    protected:
        Ref<SDFNode> m_node;
    };

    /************************** SDF Hull ******************************/

    class SDFHull final : public SDFUnaryOperator
    {
    public:
        SDFHull(const Ref<SDFNode> &n, float thickness, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFHull() = default;

        static Ref<SDFHull> create(const Ref<SDFNode> &n, float thickness, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;

        float &thickness();

    private:
        float m_thickness;
    };

    /************************** SDF Repetition ******************************/

    class SDFRepetition final : public SDFUnaryOperator
    {
    public:
        SDFRepetition(const Ref<SDFNode> &n, float t, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFRepetition() = default;

        static Ref<SDFRepetition> create(const Ref<SDFNode> &n, float t, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;

        float& t();

    private:
        float m_t; 
    };

    /************************** SDF Binary Operator ******************************/

    class SDFBinaryOperator : public SDFNode
    {
    public:
        SDFBinaryOperator(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        virtual ~SDFBinaryOperator() = default;

        virtual float value(const Point &p) const = 0;

        Ref<SDFNode> left() override;
        Ref<SDFNode> right() override;

        virtual SDFType type() const = 0;

    protected:
        Ref<SDFNode> m_left, m_right;
    };

    /************************** SDF Smooth Binary Operator ******************************/

    class SDFSmoothBinaryOperator : public SDFBinaryOperator
    {
    public:
        SDFSmoothBinaryOperator(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float k, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        virtual ~SDFSmoothBinaryOperator() = default;

        virtual float value(const Point &p) const = 0;

        virtual SDFType type() const = 0;

        float &k();

    protected:
        float m_k;
    };

    /************************** SDF Union ******************************/

    class SDFUnion final : public SDFBinaryOperator
    {
    public:
        SDFUnion(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFUnion() = default;

        static Ref<SDFUnion> create(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;
    };

    /************************** SDF Intersection ******************************/

    class SDFIntersection final : public SDFBinaryOperator
    {
    public:
        SDFIntersection(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFIntersection() = default;

        static Ref<SDFIntersection> create(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;
    };

    /************************** SDF Substraction ******************************/

    class SDFSubstraction final : public SDFBinaryOperator
    {
    public:
        SDFSubstraction(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFSubstraction() = default;

        static Ref<SDFSubstraction> create(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;
    };

    /************************** SDF XOR ******************************/

    class SDFXOR final : public SDFBinaryOperator
    {
    public:
        SDFXOR(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFXOR() = default;

        static Ref<SDFXOR> create(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;
    };

    /************************** SDF Smooth Union ******************************/

    class SDFSmoothUnion : public SDFSmoothBinaryOperator
    {
    public:
        SDFSmoothUnion(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float k, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFSmoothUnion() = default;

        static Ref<SDFSmoothUnion> create(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float k, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;
    };

    /************************** SDF Smooth Intersection ******************************/

    class SDFSmoothIntersection final : public SDFSmoothBinaryOperator
    {
    public:
        SDFSmoothIntersection(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float k, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFSmoothIntersection() = default;

        static Ref<SDFSmoothIntersection> create(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float k, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;
    };

    /************************** SDF Smooth Substraction ******************************/

    class SDFSmoothSubstraction final : public SDFSmoothBinaryOperator
    {
    public:
        SDFSmoothSubstraction(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float k, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFSmoothSubstraction() = default;

        static Ref<SDFSmoothSubstraction> create(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float k, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;
    };

    /************************** SDF Sphere ******************************/

    class SDFSphere final : public SDFNode
    {
    public:
        SDFSphere(const Point &c, float r, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFSphere() = default;

        static Ref<SDFSphere> create(const Point &c, float r, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;

        float &radius();
        float &center();

    private:
        Point m_center;
        float m_radius;
    };

    /************************** SDF Box ******************************/

    class SDFBox final : public SDFNode
    {
    public:
        SDFBox(const Point &a, const Point &b, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFBox() = default;

        static Ref<SDFBox> create(const Point &a, const Point &b, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;

        float &pmin();
        float &pmax();

    private:
        Point m_pmin, m_pmax;
    };

    /************************** SDF Plane ******************************/

    class SDFPlane final : public SDFNode
    {
    public:
        SDFPlane(const Vector &normal, float height, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFPlane() = default;

        static Ref<SDFPlane> create(const Vector &normal, float height, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;

        float &height();
        float &normal();

    private:
        Vector m_normal;
        float m_height;
    };

    /************************** SDF Torus ******************************/

    class SDFTorus final : public SDFNode
    {
    public:
        SDFTorus(float r1, float r2, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFTorus() = default;

        static Ref<SDFTorus> create(float r1, float r2, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;

        float &r();
        float &R();

    private:
        float m_R, m_r;
    };

    /************************** SDF Capsule ******************************/

    class SDFCapsule final : public SDFNode
    {
    public:
        SDFCapsule(float radius, float height, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFCapsule() = default;

        static Ref<SDFCapsule> create(float radius, float height, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;

        float &radius();
        float &height();

    private:
        float m_radius, m_height;
    };

    /************************** SDF Cylinder ******************************/

    class SDFCylinder final : public SDFNode
    {
    public:
        SDFCylinder(float radius, float height, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFCylinder() = default;

        static Ref<SDFCylinder> create(float radius, float height, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;

        float &radius();
        float &height();

    private:
        float m_radius, m_height;
    };

    /************************** SDF Translation ******************************/

    class SDFTranslation final : public SDFUnaryOperator
    {
    public:
        SDFTranslation(const Ref<SDFNode> &node, const Vector &t, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        virtual ~SDFTranslation() = default;

        static Ref<SDFTranslation> create(const Ref<SDFNode> &node, const Vector &t, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const;

        SDFType type() const;

        float &translation();

    private:
        Vector m_translation;
    };

    /************************** SDF Rotation ******************************/

    class SDFRotation : public SDFUnaryOperator
    {
    public:
        SDFRotation(const Ref<SDFNode> &node, const Vector &axis, float angle, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        virtual ~SDFRotation() = default;

        static Ref<SDFRotation> create(const Ref<SDFNode> &node, const Vector &axis, float angle, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point& p) const;

        SDFType type() const;

        float& axis(); 
        float& angle(); 

    protected: 
        Vector m_axis; 
        float m_angle; 
    };

    /************************** SDF Rotation X******************************/

    class SDFRotationX : public SDFRotation
    {
    public:
        SDFRotationX(const Ref<SDFNode> &node, float angle, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        virtual ~SDFRotationX() = default;

        static Ref<SDFRotationX> create(const Ref<SDFNode> &node, float angle, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        SDFType type() const;
    };

    /************************** SDF Rotation Y ******************************/

    class SDFRotationY : public SDFRotation
    {
    public:
        SDFRotationY(const Ref<SDFNode> &node, float angle, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        virtual ~SDFRotationY() = default;

        static Ref<SDFRotationY> create(const Ref<SDFNode> &node, float angle, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        SDFType type() const;
    };

    /************************** SDF Rotation Z ******************************/

    class SDFRotationZ : public SDFRotation
    {
    public:
        SDFRotationZ(const Ref<SDFNode> &node, float angle, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        virtual ~SDFRotationZ() = default;

        static Ref<SDFRotationZ> create(const Ref<SDFNode> &node, float angle, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        SDFType type() const;
    };

    /************************** SDF Scale ******************************/

    class SDFScale final : public SDFUnaryOperator
    {
    public:
        SDFScale(const Ref<SDFNode> &node, float s, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        virtual ~SDFScale() = default;

        static Ref<SDFScale> create(const Ref<SDFNode> &node, float s, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const;

        float &scale();

    private:
        float m_scale;
    };

    /************************** SDF Tree ******************************/

    class SDFTree final : public SDFNode
    {
    public:
        SDFTree(const Ref<SDFNode> &root = nullptr, float l = 0.f, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        static Ref<SDFTree> create(const Ref<SDFNode> &root = nullptr, float l = 0.f, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        Ref<SDFNode> left() override;
        Ref<SDFNode> right() override;

        SDFType type() const override;
        std::vector<SDFType> tree_type() const;

        Ref<Mesh> polygonize(int resolution, const Box &box) const;
        Vector normal(const Vector &) const;
        Vector dichotomy(Vector, Vector, float, float, float) const;

        void root(const Ref<SDFNode> &node);
        Ref<SDFNode> &root();

    private:
        std::vector<SDFType> tree_type(const Ref<SDFNode> &node) const;

    private:
        static int s_triangle_table[256][16]; //!< Two dimensionnal array storing the straddling edges for every marching cubes configuration.
        static int s_edge_table[256];         //!< Array storing straddling edges for every marching cubes configuration.

    private:
        Ref<SDFNode> m_root;
    };

    const char *type_str(SDFType type);

} // namespace gm
