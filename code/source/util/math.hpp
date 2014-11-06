#ifndef CLOVER_UTIL_MATH_HPP
#define CLOVER_UTIL_MATH_HPP

#include "build.hpp"
#include "dyn_array.hpp"
#include "math_constants.hpp"
#include "math_utils.hpp"
#include "vector.hpp"
#include "matrix.hpp"
#include "quaternion.hpp"

/// @todo Remove box2d from here
#include <Box2D/Box2D.h>
#include <iostream>
#include <random>

namespace clover {
namespace util {
namespace geom {

util::DynArray<util::Vec2d> circleLineIntersection(util::Vec2d circlepos, real32 radius, util::Vec2d line1, util::Vec2d line2);
util::DynArray<util::Vec2d> circleEdgeIntersection(util::Vec2d circlepos, real32 radius, util::Vec2d edge1, util::Vec2d edge2);

/// Untested
util::DynArray<util::Vec2d> edgeEdgeIntersection(util::Vec2d a1, util::Vec2d a2, util::Vec2d b1, util::Vec2d b2);

template <typename T> // util::Vec2f or util::Vec2d
bool isPointInsideTriangle(const T& p, const T& a, const T& b, const T& c);

template <typename T> // util::Vec2f or util::Vec2d
util::Vec3d barycentric(const T& p, const T& a, const T& b, const T& c);

real32 polygonArea(const util::DynArray<util::Vec2d>& contour);

/// p0 ja p3 are control points
template <typename T> // util::Vec2f or util::Vec2d
T pointOnCatmull(real64 t, T p0, T p1, T p2, T p3);

} // geom

/// @todo Shouldn't be a class
class Rand {
public:
	template<typename T>
	static T continuous(T min, T max){
		std::uniform_real_distribution<T> dist(min, max);
		return dist(generator);
	}

	template<typename T>
	static T discrete(T min, T max){
		std::uniform_int_distribution<T> dist(min, max);
		return dist(generator);
	}

private:
	static std::mt19937 generator;
};

int32 limitC(int32 i);
int32 limitUC(int32 i);

template <typename T>
T limited(T f, T ala, T yla){
	if (f < ala) return ala;
	if (f > yla) return yla;
	return f;
}

real32 normalizedAngle(real32 angle);

real32 angleBetweenCW(real32 angle1, real32 angle2);
real32 angleBetweenCCW(real32 angle1, real32 angle2);

real32 smallestAngleBetween(real32 angle1, real32 angle2);

bool isPointInFrontOfLine(util::Vec2d point, util::Vec2d line_anchor, util::Vec2d normal);
util::Vec2d closestPointOnLineSegment(util::Vec2d point, util::Vec2d a, util::Vec2d b);

} // util
} // clover

#endif // CLOVER_UTIL_MATH_HPP