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
        PRIMITIVE_SPHERE = 0,
        PRIMITIVE_BOX,
        PRIMITIVE_ROUND_BOX,
        PRIMITIVE_TORUS,
        PRIMITIVE_CYLINDER,
        PRIMITIVE_CONE,
        PRIMITIVE_PLANE,
        PRIMITIVE_CAPSULE,
        UNARY_OPERATOR_HULL,
        BINARY_OPERATOR_UNION,
        BINARY_OPERATOR_SMOOTH_UNION,
        BINARY_OPERATOR_INTERSECTION,
        BINARY_OPERATOR_SMOOTH_INTERSECTION,
        BINARY_OPERATOR_SUBSTRACTION,
        BINARY_OPERATOR_SMOOTH_SUBSTRACTION,
        TREE,
        NB_ELT
    };

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

        virtual SDFType type() const = 0;

    private:
        bool intersect_ray_marching(const Ray &ray, float eps = 1e-3) const;
        bool intersect_sphere_tracing(const Ray &ray, float t) const;

    protected:
        static const float s_epsilon; //!< Epsilon value for partial derivatives
        static const int s_limit;     //!< Epsilon value for intersection limit
    protected:
        float m_lambda{1.0};

        IntersectMethod m_intersect_method;
    };

    class SDFUnaryOperator : public SDFNode
    {
    public:
        SDFUnaryOperator(const Ref<SDFNode> &n);
        ~SDFUnaryOperator() = default;

        virtual float value(const Point &p) const = 0;

        Ref<SDFNode> left() override; 
        Ref<SDFNode> right() override; 

        virtual SDFType type() const = 0;
    
    protected:
        Ref<SDFNode> m_node;
    };

    class SDFHull final : public SDFUnaryOperator
    {
    public:
        SDFHull(const Ref<SDFNode> &n, float thickness);
        ~SDFHull() = default;

        static Ref<SDFHull> create(const Ref<SDFNode> &n, float thickness);

        float value(const Point &p) const override;

        SDFType type() const override;

        float& thickness(); 

    private:
        float m_thickness;
    };

    class SDFBinaryOperator : public SDFNode
    {
    public:
        SDFBinaryOperator(const Ref<SDFNode> &l, const Ref<SDFNode> &r);
        ~SDFBinaryOperator() = default;

        virtual float value(const Point &p) const = 0;

        Ref<SDFNode> left() override; 
        Ref<SDFNode> right() override; 

        virtual SDFType type() const = 0;

    protected:
        Ref<SDFNode> m_left, m_right;
    };

    class SDFSmoothBinaryOperator : public SDFBinaryOperator
    {
    public:
        SDFSmoothBinaryOperator(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float k);
        ~SDFSmoothBinaryOperator() = default;

        virtual float value(const Point &p) const = 0;

        virtual SDFType type() const = 0;

        float& k();

    protected:
        float m_k;
    };

    class SDFUnion final : public SDFBinaryOperator
    {
    public:
        SDFUnion(const Ref<SDFNode> &l, const Ref<SDFNode> &r);
        ~SDFUnion() = default;

        static Ref<SDFUnion> create(const Ref<SDFNode> &l, const Ref<SDFNode> &r);

        float value(const Point &p) const override;

        SDFType type() const override;
    };

    class SDFIntersection final : public SDFBinaryOperator
    {
    public:
        SDFIntersection(const Ref<SDFNode> &l, const Ref<SDFNode> &r);
        ~SDFIntersection() = default;

        static Ref<SDFIntersection> create(const Ref<SDFNode> &l, const Ref<SDFNode> &r);

        float value(const Point &p) const override;

        SDFType type() const override;
    };

    class SDFSubstraction final : public SDFBinaryOperator
    {
    public:
        SDFSubstraction(const Ref<SDFNode> &l, const Ref<SDFNode> &r);
        ~SDFSubstraction() = default;

        static Ref<SDFSubstraction> create(const Ref<SDFNode> &l, const Ref<SDFNode> &r);

        float value(const Point &p) const override;

        SDFType type() const override;
    };

    class SDFSmoothUnion : public SDFSmoothBinaryOperator
    {
    public:
        SDFSmoothUnion(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float k);
        ~SDFSmoothUnion() = default;

        static Ref<SDFSmoothUnion> create(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float k);

        float value(const Point &p) const override;

        SDFType type() const override;
    };

    class SDFSmoothIntersection final : public SDFSmoothBinaryOperator
    {
    public:
        SDFSmoothIntersection(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float k);
        ~SDFSmoothIntersection() = default;

        static Ref<SDFSmoothIntersection> create(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float k);

        float value(const Point &p) const override;

        SDFType type() const override;
    };

    class SDFSmoothSubstraction final : public SDFSmoothBinaryOperator
    {
    public:
        SDFSmoothSubstraction(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float k);
        ~SDFSmoothSubstraction() = default;

        static Ref<SDFSmoothSubstraction> create(const Ref<SDFNode> &l, const Ref<SDFNode> &r, float k);

        float value(const Point &p) const override;

        SDFType type() const override;
    };

    class SDFSphere final : public SDFNode
    {
    public:
        SDFSphere(const Point &c, float r, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFSphere() = default;

        static Ref<SDFSphere> create(const Point &c, float r, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;

        float& radius();

    private:
        Point m_center;
        float m_radius;
    };

    class SDFBox final : public SDFNode
    {
    public:
        SDFBox(const Point &a, const Point &b, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFBox() = default;

        static Ref<SDFBox> create(const Point &a, const Point &b, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;

        float& pmin();
        float& pmax();

    private:
        Point m_pmin, m_pmax;
    };

    class SDFPlane final : public SDFNode
    {
    public:
        SDFPlane(const Vector &normal, float h, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFPlane() = default;

        static Ref<SDFPlane> create(const Vector &normal, float h, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;

        float& h();
        float& normal();

    private:
        Vector m_normal;
        float m_h;
    };

    class SDFTorus final : public SDFNode
    {
    public:
        SDFTorus(float r1, float r2, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~SDFTorus() = default;

        static Ref<SDFTorus> create(float r1, float r2, float l = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);

        float value(const Point &p) const override;

        SDFType type() const override;

        float& r();
        float& R();

    private:
        float m_R, m_r;
    };

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
        Ref<SDFNode>& root();

    private: 
        std::vector<SDFType> tree_type(const Ref<SDFNode>& node) const;

    private:
        static int m_triangle_table[256][16]; //!< Two dimensionnal array storing the straddling edges for every marching cubes configuration.
        static int m_edge_table[256];         //!< Array storing straddling edges for every marching cubes configuration.

    private:
        Ref<SDFNode> m_root;
    };

    const char *type_str(SDFType type);

} // namespace gm
