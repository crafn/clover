#ifndef CLOVER_PHYSICS_FRICTION_HPP
#define CLOVER_PHYSICS_FRICTION_HPP

#include "build.hpp"

namespace clover {
namespace physics {

class Object;

/// Applies forces to bodies which drive them towards configuration
/// in which they move like a single rigid body
void applyFriction(	Object& a, Object& b,
					real64 friction_mul, real64 dt);

} // physics
} // clover

#endif // CLOVER_PHYSICS_FRICTION_HPP
