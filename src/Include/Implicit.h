#pragma once

#include "pch.h"

#include "Box.h"

/*
SDF point -> line AB

h= min(1, max(0, dot(ap, ab)/dot(ab, ab)))
d= length(P - A - h * ab)

*/

template<typename T>
using Ref = std::shared_ptr<T>;

namespace gm
{
    struct Ray
    {
        Point origin { 0, 0, 0 }; 
        Vector direction { 0, 0, 0 }; 

        Point point(float t) const;
    };

    enum class IntersectMethod 
    {
        RAY_MARCHING=0, 
        SPHERE_TRACING, 
        NB_ELT 
    };

    enum class ImplicitType 
    {
        PRIMITIVE_SPHERE=0, 
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
        BINARY_OPERATOR_DIFFERENCE, 
        BINARY_OPERATOR_SMOOTH_DIFFERENCE, 
        TREE,
        NB_ELT 
    };


    class ImplicitNode
    {
    public:
        ImplicitNode(float lambda= 1.0, IntersectMethod method= IntersectMethod::RAY_MARCHING);
        virtual ~ImplicitNode()=default;

        virtual float value(const Point &p) const;
        virtual bool inside(const Point &p) const;
        virtual Vector gradient(const Point &p) const;
        virtual bool intersect(const Ray &ray, float eps) const;

        void intersect_method(IntersectMethod method);

        virtual ImplicitType type() const=0;

    private: 
        bool intersect_ray_marching(const Ray &ray, float eps= 1e-3) const;
        bool intersect_sphere_tracing(const Ray &ray, float t) const;

    protected: 
        static const float s_epsilon; //!< Epsilon value for partial derivatives
        static const int s_limit; //!< Epsilon value for intersection limit
    protected:
        float m_lambda { 1.0 };

        IntersectMethod m_intersect_method; 
    };

    class ImplicitUnaryOperator : public ImplicitNode
    {
    public:
        ImplicitUnaryOperator(const Ref<ImplicitNode>& n);
        ~ImplicitUnaryOperator()=default;

        virtual float value(const Point& p) const=0;

        virtual ImplicitType type() const=0;

    protected:
        Ref<ImplicitNode> m_node;
    };

    class ImplicitHull final : public ImplicitUnaryOperator
    {
    public:
        ImplicitHull(const Ref<ImplicitNode>& n, float thickness);
        ~ImplicitHull()=default;

        static Ref<ImplicitHull> create(const Ref<ImplicitNode>& n, float thickness);

        float value(const Point& p) const override;

        ImplicitType type() const override;

    private: 
        float m_thickness; 
    };

    class ImplicitBinaryOperator : public ImplicitNode
    {
    public:
        ImplicitBinaryOperator(const Ref<ImplicitNode>& l, const Ref<ImplicitNode>& r);
        ~ImplicitBinaryOperator()=default;

        virtual float value(const Point& p) const=0;

        virtual ImplicitType type() const=0;

    protected:
        Ref<ImplicitNode> m_left, m_right;
    };

    class ImplicitUnion final : public ImplicitBinaryOperator
    {   
    public: 
        ImplicitUnion(const Ref<ImplicitNode>& l, const Ref<ImplicitNode>& r); 
        ~ImplicitUnion()=default; 

        static Ref<ImplicitUnion> create(const Ref<ImplicitNode>& l, const Ref<ImplicitNode>& r);

        float value(const Point& p) const override;

        ImplicitType type() const override;
    };

    class ImplicitIntersection final : public ImplicitBinaryOperator
    {   
    public: 
        ImplicitIntersection(const Ref<ImplicitNode>& l, const Ref<ImplicitNode>& r); 
        ~ImplicitIntersection()=default; 

        static Ref<ImplicitIntersection> create(const Ref<ImplicitNode>& l, const Ref<ImplicitNode>& r);

        float value(const Point& p) const override;

        ImplicitType type() const override;
    };

    class ImplicitDifference final : public ImplicitBinaryOperator
    {   
    public: 
        ImplicitDifference(const Ref<ImplicitNode>& l, const Ref<ImplicitNode>& r); 
        ~ImplicitDifference()=default; 

        static Ref<ImplicitDifference> create(const Ref<ImplicitNode>& l, const Ref<ImplicitNode>& r);

        float value(const Point& p) const override;

        ImplicitType type() const override;
    };

    class ImplicitSphere final: public ImplicitNode 
    {
    public: 
        ImplicitSphere(const Point& c, float r, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~ImplicitSphere()=default;

        static Ref<ImplicitSphere> create(const Point& c, float r, float l= 1.0, IntersectMethod im= IntersectMethod::RAY_MARCHING);

        float value(const Point& p) const override; 

        ImplicitType type() const override;
        
    private: 
        Point m_center; 
        float m_radius;
    };  

    class ImplicitBox final: public ImplicitNode 
    {
    public: 
        ImplicitBox(const Point& a, const Point& b, float lambda = 1.0, IntersectMethod im = IntersectMethod::RAY_MARCHING);
        ~ImplicitBox()=default;

        static Ref<ImplicitBox> create(const Point& a, const Point& b, float l= 1.0, IntersectMethod im= IntersectMethod::RAY_MARCHING);

        float value(const Point& p) const override; 

        ImplicitType type() const override;
        
    private: 
        Point m_pmin, m_pmax; 
    };  


    class ImplicitTree final: public ImplicitNode
    {
    public:
        ImplicitTree(const Ref<ImplicitNode>& root=nullptr, float l= 0.f, IntersectMethod im= IntersectMethod::RAY_MARCHING);

        static Ref<ImplicitTree> create(const Ref<ImplicitNode>& root=nullptr, float l= 0.f, IntersectMethod im= IntersectMethod::RAY_MARCHING);

        float value(const Point& p) const override;
        
        ImplicitType type() const override;

        Mesh polygonize(int resolution, const Box& box) const;
        Vector normal(const Vector &) const;
        Vector dichotomy(Vector, Vector, float, float, float) const;


    private:
        static int m_triangle_table[256][16]; //!< Two dimensionnal array storing the straddling edges for every marching cubes configuration.
        static int m_edge_table[256];         //!< Array storing straddling edges for every marching cubes configuration.

    private:
        Ref<ImplicitNode> m_root; 
    };
} // namespace gm
