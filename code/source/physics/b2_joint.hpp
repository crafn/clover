#ifndef CLOVER_PHYSICS_B2_JOINT_HPP
#define CLOVER_PHYSICS_B2_JOINT_HPP

#include "build.hpp"
#include "joint.hpp"
#include "world.hpp"
#include "util/vector.hpp"

namespace clover {
namespace physics {

/// Internal class of the physics system
template <typename DefType, typename JointType>
class B2Joint : public Joint {
public:
	virtual ~B2Joint(){ detach(); }
	virtual bool isAttached() const override { return joint; }
	virtual void detach();

	virtual WorldVec getAnchor(SizeType i) const override;

	JointType* getB2Joint() const { return joint; }

protected:
	DefType def;
	JointType* joint= nullptr;

	/// Object calls
	virtual void recreate() override;

	virtual void createB2Joint();
	virtual void destroyB2Joint();

private:
	b2Body* getB2Body(SizeType i) const;
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_B2_JOINT_HPP