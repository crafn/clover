#ifndef CLOVER_COLLISION_RAY_HPP
#define CLOVER_COLLISION_RAY_HPP

#include "build.hpp"
#include "util/vector.hpp"

namespace clover {
namespace collision {

struct Ray {
	Ray(const util::Vec2d& start_pos, const util::Vec2d& end_offset):
		start(start_pos), endOffset(end_offset){}

	util::Vec2d start;
	util::Vec2d endOffset;
};

struct RayCastResult {
	RayCastResult():fraction(-1){}

	util::Vec2d normal;
	real64 fraction;
};

} // collision
} // clover

#endif // CLOVER_COLLISION_RAY_HPP