#ifndef CLOVER_NODEINSTANCE_WE_ATTACHEDPHYSICSOBJECT_HPP
#define CLOVER_NODEINSTANCE_WE_ATTACHEDPHYSICSOBJECT_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"
#include "physics/joint_hardweld.hpp"
#include "physics/object_rigid.hpp"
#include "util/dynamic.hpp"
#include "util/unique_ptr.hpp"

namespace clover {
namespace nodes {

class WeAttachedPhysicsObjectNodeInstance : public NodeInstance {
public:
	virtual void create();
	virtual void update();

private:
	void attach();
	void onDetach();
	bool objectCanMove() const;

	InputSlot<SignalType::Boolean>* activeIn;
	InputSlot<SignalType::SrtTransform3>* transformIn;
	InputSlot<SignalType::Vec2>* anchorIn;
	InputSlot<SignalType::WeHandle>* weInput;

	OutputSlot<SignalType::SrtTransform3>* transformOut;
	OutputSlot<SignalType::SrtTransform3>* estimatedOut;
	OutputSlot<SignalType::Trigger>* detachedOut;
	
	util::UniquePtr<physics::RigidObject> object;
	util::Dynamic<physics::HardWeldJoint> joint;
	util::CbListener<physics::OnDetachCb> detachListener;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::WeAttachedPhysicsObjectNodeInstance> {
	static util::Str8 type(){ return "::WeAttachedPhysicsObjectNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODEINSTANCE_WE_ATTACHEDPHYSICSOBJECT_HPP
