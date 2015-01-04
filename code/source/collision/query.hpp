#ifndef CLOVER_COLLISION_QUERY_HPP
#define CLOVER_COLLISION_QUERY_HPP

#include "build.hpp"
#include "collision/traceable.hpp"
#include "collision/ray.hpp"
#include "physics/b2_fixtureuserdata.hpp"
#include "physics/fixture_rigid.hpp"
#include "util/vector.hpp"

namespace clover {
namespace collision {

/// For making derived traceable queries shorter
template <typename T>
struct SubQuery {

	static void rayCast(const Ray& ray, std::function<real64 (T&, const RayCastResult&)> report);
	static void potentialRect(const util::Vec2d& pos, const util::Vec2d& rad, std::function<bool (T&)> report);
	static void point(const util::Vec2d& pos, std::function<bool (T&)> report);
};

class Query {
public:
	/// Enum-like special return values for rayCast report-function
	struct Fraction {
		static constexpr real64 Ignore= -1.0;
		static constexpr real64 Terminate= 0.0;
		static constexpr real64 Continue= 1.0;

	};

	/// Performs a ray-cast. Report -function works as callbacks in Box2D:
	/**
		You control the continuation of the ray cast by returning a fraction. Returning a fraction of Fraction::Terminate indicates
		the ray cast should be terminated. A fraction of Fraction::Continue indicates the ray cast should continue as if no hit occurred.
		If you return the fraction from the argument list, the ray will be clipped to the current intersection point. So
		you can ray cast any shape, ray cast all shapes, or ray cast the closest shape by returning the appropriate fraction.
		You may also return of fraction of Fraction::Ingore to filter the fixture. Then the ray cast will proceed as if the fixture does not exist.
	**/

	static void rayCast(const Ray& ray, std::function<real64 (Traceable&, const RayCastResult&)> report);

	/// Calls report for all Traceables that potentially overlap the rect (until report returns false)
	/// Use this if there's a good reason to use rect(..)
	static void potentialRect(const util::Vec2d& pos, const util::Vec2d& rad, std::function<bool (Traceable&)> report);


	/// Calls report for all Traceables under a point (until report returns false)
	static void point(const util::Vec2d& pos, std::function<bool (Traceable&)> report);


	/// These work like normal queries, but use the template parameter instead of Traceable
	static SubQuery<physics::Fixture> fixture;
	static SubQuery<physics::RigidFixture> rigidFixture;
};


template <typename T>
void SubQuery<T>::rayCast(const Ray& ray, std::function<real64 (T&, const RayCastResult&)> report){
	auto lambda= [&] (Traceable& t, const RayCastResult& result) -> real64 {
		if (t.isConvertibleTo(T::staticTraceableTypeId))
			return report(*static_cast<T*>(&t), result);
		return Query::Fraction::Ignore;
	};

	Query::rayCast(ray, lambda);
}

template <typename T>
void SubQuery<T>::potentialRect(const util::Vec2d& pos, const util::Vec2d& rad, std::function<bool (T&)> report){
	auto lambda= [&] (Traceable& t) -> bool {
		if (t.isConvertibleTo(T::staticTraceableTypeId))
			return report(*static_cast<T*>(&t));
		return true;
	};

	Query::potentialRect(pos, rad, lambda);
}

template <typename T>
void SubQuery<T>::point(const util::Vec2d& pos, std::function<bool (T&)> report){
	auto lambda= [&] (Traceable& t) -> bool {
		if (t.isConvertibleTo(T::staticTraceableTypeId))
			return report(*static_cast<T*>(&t));
		return true;
	};

	Query::point(pos, lambda);
}

} // collision
} // clover

#endif // CLOVER_COLLISION_QUERY_HPP
