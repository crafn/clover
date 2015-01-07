#include "box2d.hpp"
#include "global/env.hpp"
#include "physics/phys_mgr.hpp"
#include "physics/world.hpp"
#include "query.hpp"

namespace clover {
namespace collision {

template <typename Func>
class RayCallback : public b2RayCastCallback {
public:
	RayCallback(Func& f): func(f) {}

	Func& func;

	real64 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, real64 fraction){
		ensure(fixture);
		physics::B2FixtureUserData* user_data= static_cast<physics::B2FixtureUserData*>(fixture->GetUserData());
		ensure(user_data->owner);

		RayCastResult output;
		output.normal= fromB2(normal);
		output.fraction= fraction;
		return func(*user_data->owner, output);
	}
};

template <typename Func>
class RectCallback : public b2QueryCallback {
public:
	RectCallback(Func& f): func(f) {}

	Func& func;

	bool ReportFixture(b2Fixture* fixture){
		ensure(fixture);
		physics::B2FixtureUserData* user_data= static_cast<physics::B2FixtureUserData*>(fixture->GetUserData());
		ensure(user_data->owner);

		return func(*user_data->owner);
	}
};

void Query::rayCast(const Ray& ray, std::function<real64 (Traceable&, const RayCastResult&)> report){
	RayCallback<decltype(report)> callback(report);
	global::g_env.physMgr->getWorld().getB2World().RayCast(&callback, toB2(ray.start), toB2(util::Vec2d(ray.start+ray.endOffset)));
}

void Query::potentialRect(const util::Vec2d& pos, const util::Vec2d& rad, std::function<bool (Traceable&)> report){
	RectCallback<decltype(report)> callback(report);
	b2AABB aabb;
	aabb.lowerBound= toB2(pos-rad);
	aabb.upperBound= toB2(pos+rad);
	global::g_env.physMgr->getWorld().getB2World().QueryAABB(&callback, aabb);
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
