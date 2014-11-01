#ifndef CLOVER_COLLISION_BASESHAPE_POLYGON_HPP
#define CLOVER_COLLISION_BASESHAPE_POLYGON_HPP

#include "build.hpp"
#include "baseshape.hpp"
#include "util/dyn_array.hpp"

#include <Box2D/Box2D.h>

namespace clover {
namespace collision {

/// A simple polygon
class PolygonBaseShape : public BaseShape {
public:
	/// Box2d doesn't like too small polygons
	static constexpr real64 minArea= 0.0001;
	/// Or too short edges
	static constexpr real64 minEdgeLength= 0.0001;
	static constexpr SizeType maxInternalVertexCount= b2_maxPolygonVertices;

	PolygonBaseShape(const util::DynArray<util::Vec2d>& v = {});
	virtual ~PolygonBaseShape();

	void setVertices(util::DynArray<util::Vec2d> v);
	util::DynArray<util::Vec2d> getVertices() const;
	void set(const util::Polygon& p);
	void setAsRect(const util::Vec2d& pos, const util::Vec2d& rad, real64 angle=0);

	virtual void transform(const Transform& t) override;
	virtual void mirror() override;
	virtual const b2Shape& getB2Shape(SizeType i) const override;
	virtual util::Polygon asPolygon(real64 imprecision) const override;
	virtual util::DynArray<util::Polygon>
		asConvexPolygons(real64 imprecision) const override;
	virtual PolygonBaseShape* clone() const;

private:
	util::Polygon poly;
	util::DynArray<b2PolygonShape> shapes;
};

} // collision
namespace util {

template <>
struct TypeStringTraits<collision::PolygonBaseShape> {
	static util::Str8 type(){ return "collision::PolygonBaseShape"; }
};

} // util
} // clover

#endif // CLOVER_COLLISION_BASESHAPE_POLYGON_HPP
