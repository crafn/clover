#ifndef CLOVER_COLLISION_BASESHAPE_HPP
#define CLOVER_COLLISION_BASESHAPE_HPP

#include "build.hpp"
#include "collision/ray.hpp"
#include "util/dyn_array.hpp"
#include "util/objectnodetraits.hpp"
#include "util/polygon.hpp"
#include "util/transform.hpp"

#include <memory>

class b2Shape;

namespace clover {
namespace collision {

/// Abstract collision shape
/// @todo Using abstraction and virtual functions here is just stupid.
class BaseShape {
public:
	typedef util::RtTransform<real64, util::Vec2d> Transform;

	virtual ~BaseShape(){}

	bool overlaps(	const collision::Ray& ray,
					const Transform& transform,
					collision::RayCastResult* output= nullptr) const;
	bool overlaps(	const util::Vec2d& pos,
					const Transform& transform) const;
	bool overlaps(	const BaseShape& shape,
					const Transform& shape_transform,
					const Transform& transform) const;
	real64 distance(const BaseShape& shape,
					const Transform& shape_transform,
					const Transform& transform) const;

	SizeType getB2ShapeCount() const { return b2ShapeCount; }

	virtual void transform(const Transform& t)= 0;
	/// Mirrors along x-axis
	virtual void mirror()= 0;
	virtual util::Polygon asPolygon(real64 imprecision) const = 0;
	virtual util::DynArray<util::Polygon>
		asConvexPolygons(real64 imprecision) const = 0;
	virtual const b2Shape& getB2Shape(SizeType i) const = 0;
	virtual BaseShape* clone() const = 0;

protected:
	// Derived sets this
	SizeType b2ShapeCount= 0;
};

} // collision
namespace util {

template <>
struct ObjectNodeTraits<std::shared_ptr<collision::BaseShape>> {
	typedef std::shared_ptr<collision::BaseShape> Value;
	static util::ObjectNode serialized(const Value& value);
	static Value deserialized(const util::ObjectNode& ob_node);
};

} // util
} // clover

#endif // CLOVER_COLLISION_BASESHAPE_HPP
