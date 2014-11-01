#include "query.hpp"

namespace clover {
namespace collision {

void Query::rayCast(const Ray& ray, std::function<real64 (Traceable&, const RayCastResult&)> report){
	RayCallback<decltype(report)> callback(report);
	physics::gWorld->getB2World().RayCast(&callback, ray.start.b2(), util::Vec2d(ray.start+ray.endOffset).b2());
}

void Query::potentialRect(const util::Vec2d& pos, const util::Vec2d& rad, std::function<bool (Traceable&)> report){
	RectCallback<decltype(report)> callback(report);
	b2AABB aabb;
	aabb.lowerBound= util::Vec2d(pos-rad).b2();
	aabb.upperBound= util::Vec2d(pos+rad).b2();
	physics::gWorld->getB2World().QueryAABB(&callback, aabb);
}

void Query::point(const util::Vec2d& pos, std::function<bool (Traceable&)> report){
	auto lambda= [&] (Traceable& t) {
		if (t.overlaps(pos)) return report(t);
		return true;
	};
	potentialRect(pos, util::Vec2d(0.01), lambda);
}

} // collision
} // clover