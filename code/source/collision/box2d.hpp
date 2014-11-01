#ifndef CLOVER_COLLISION_BOX2D_HPP
#define CLOVER_COLLISION_BOX2D_HPP

#include "util/vector.hpp"
#include "util/transform.hpp"

#include <Box2D/Box2D.h>

namespace clover {
namespace collision {

inline
b2Vec2 toB2(util::Vec2d v){ return b2Vec2{v.x, v.y}; }

inline
util::Vec2d fromB2(b2Vec2 v){ return util::Vec2d{v.x, v.y}; }

inline
b2Transform toB2(util::RtTransform<real64, util::Vec2d> t)
{ return b2Transform{toB2(t.translation), b2Rot{t.rotation}}; }

} // collision
} // clover

#endif // CLOVER_COLLISION_BOX2D_HPP
