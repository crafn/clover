#ifndef CLOVER_COLLISION_BASESHAPE_CIRCLE_HPP
#define CLOVER_COLLISION_BASESHAPE_CIRCLE_HPP

#include "build.hpp"
#include "baseshape.hpp"

#include <Box2D/Collision/Shapes/b2CircleShape.h>

namespace clover {
namespace collision {

ENGINE_API util::DynArray<util::Vec2d>
circleVertices(util::Vec2d center, real64 radius, int32 vcount);

class ENGINE_API CircleBaseShape : public BaseShape {
public:
	CircleBaseShape(const util::Vec2d& center= util::Vec2d(0), real64 r=1.0);
	virtual ~CircleBaseShape();

	void setRadius(real64 r){ shape.m_radius= r; }
	real64 getRadius() const { return shape.m_radius; }
	
	void setPosition(const util::Vec2d& pos);
	util::Vec2d getPosition() const;

	virtual void transform(const Transform& t) override;
	virtual void mirror() override;
	virtual const b2Shape& getB2Shape(SizeType i) const override;
	virtual util::Polygon asPolygon(real64 imprecision) const override;
	virtual util::DynArray<util::Polygon>
		asConvexPolygons(real64 imprecision) const override;
	virtual CircleBaseShape* clone() const override;
	
private:
	b2CircleShape shape;
};

} // collision
namespace util {

template <>
struct TypeStringTraits<collision::CircleBaseShape> {
	static util::Str8 type(){ return "collision::CircleBaseShape"; }
};

} // util
} // clover

#endif // CLOVER_COLLISION_BASESHAPE_CIRCLE_HPP
