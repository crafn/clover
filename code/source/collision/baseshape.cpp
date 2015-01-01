#include "baseshape.hpp"
#include "box2d.hpp"
#include "collision/baseshape_circle.hpp"
#include "collision/baseshape_polygon.hpp"
#include "collision/box2d.hpp"
#include "resources/exception.hpp"
#include "util/dyn_array.hpp"
#include "util/vector.hpp"

#include <Box2D/Box2D.h>

namespace clover {
namespace collision {

bool BaseShape::overlaps(const util::Vec2d& pos, const Transform& transform) const {
	b2Transform t;
	t.Set(toB2(transform.translation), transform.rotation);

	for (SizeType i= 0; i < b2ShapeCount; ++i){
		if (getB2Shape(i).TestPoint(t, toB2(pos)))
			return true;
	}

	return false;
}

bool BaseShape::overlaps(const collision::Ray& ray, const Transform& transform, collision::RayCastResult* output) const {
	b2Transform b2_t= toB2(transform);

	b2RayCastInput b2_input;
	b2_input.p1= toB2(ray.start);
	b2_input.p2= toB2(ray.start + ray.endOffset);
	b2_input.maxFraction= 1.0;

	bool found= false;
	if (output){
		output->fraction= 1.0;
	}

	b2RayCastOutput b2_output;

	for (SizeType i= 0; i < b2ShapeCount; ++i){
		if (getB2Shape(i).RayCast(&b2_output, b2_input, b2_t, 0)){
			if (output && b2_output.fraction <= output->fraction){
				output->normal= fromB2(b2_output.normal);
				output->fraction= b2_output.fraction;
				found= true;
			}
		}
	}

	return found;
}

bool BaseShape::overlaps(	const BaseShape& shape,
							const Transform& shape_transform,
							const Transform& transform) const {
	b2Transform a= toB2(transform);
	b2Transform	b= toB2(shape_transform);

	for (SizeType i= 0; i < b2ShapeCount; ++i){
		for (SizeType k= 0; k < shape.getB2ShapeCount(); ++k){
			if (b2TestOverlap(&getB2Shape(i), 0, &shape.getB2Shape(k), 0, a, b))
				return true;
		}
	}

	return false;
}

real64 BaseShape::distance(	const BaseShape& shape,
							const Transform& shape_transform,
							const Transform& transform) const {
	real64 closest_dist= std::numeric_limits<real64>::max();
	for (SizeType i= 0; i < b2ShapeCount; ++i){
		for (SizeType k= 0; k < shape.getB2ShapeCount(); ++k){
			b2DistanceInput in;
			in.transformA= toB2(transform);
			in.transformB= toB2(shape_transform);
			in.proxyA.Set(&getB2Shape(i), 0);
			in.proxyB.Set(&shape.getB2Shape(k), 0);
			in.useRadii= true;

			b2DistanceOutput out;
			b2SimplexCache cache;
			cache.count= 0;
			b2Distance(&out, &cache, &in);

			closest_dist= std::min(closest_dist, out.distance);
		}
	}
	return closest_dist;
}

} // collision
namespace util {

util::ObjectNode ObjectNodeTraits<std::shared_ptr<collision::BaseShape>>::serialized(const Value& value){
	util::ObjectNode ret;

	collision::CircleBaseShape* circle= dynamic_cast<collision::CircleBaseShape*>(value.get());
	collision::PolygonBaseShape* poly= dynamic_cast<collision::PolygonBaseShape*>(value.get());

	ensure(circle || poly);

	if (circle){
		ret["type"].setValue<util::Str8>("Circle");
		ret["center"]= util::ObjectNode::create<util::Vec2d>(circle->getPosition());
		ret["radius"]= util::ObjectNode::create<real64>(circle->getRadius());
	}
	else if (poly){
		ret["type"].setValue<util::Str8>("Polygon");
		ret["vertices"]= util::ObjectNode::create<util::DynArray<util::Vec2d>>(poly->getVertices());
	}

	return (ret);
}

auto ObjectNodeTraits<std::shared_ptr<collision::BaseShape>>::deserialized(const util::ObjectNode& ob_node) -> Value {
	if (ob_node.get("type").getValue<util::Str8>() == "Circle"){

		util::Vec2d pos= ob_node.get("center").getValue<util::Vec2d>();
		real64 rad= ob_node.get("radius").getValue<real64>();
		return (Value(new collision::CircleBaseShape(pos, rad)));
	}
	else if (ob_node.get("type").getValue<util::Str8>() == "Polygon"){
		util::DynArray<util::Vec2d> vert= std::move(ob_node.get("vertices").getValue<util::DynArray<util::Vec2d>>());

		return (Value(new collision::PolygonBaseShape(vert)));
	}

	throw resources::ResourceException("Invalid shape shape type");
}

} // util
} // clover
