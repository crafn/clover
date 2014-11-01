#ifndef CLOVER_PHYSICS_B2_FIXTUREUSERDATA_HPP
#define CLOVER_PHYSICS_B2_FIXTUREUSERDATA_HPP

#include "build.hpp"

namespace clover {
namespace physics {

class Fixture;
class RigidFixture;

struct B2FixtureUserData {
	B2FixtureUserData():owner(nullptr){}
	Fixture* owner;
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_B2_FIXTUREUSERDATA_HPP
