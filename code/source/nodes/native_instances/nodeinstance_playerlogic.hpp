#ifndef CLOVER_NODES_NODEINSTANCE_PLAYERLOGIC_HPP
#define CLOVER_NODES_NODEINSTANCE_PLAYERLOGIC_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"
#include "physics/armaturesuit.hpp"
#include "physics/contact.hpp"
#include "physics/entity.hpp"
#include "physics/joint_wheel.hpp"
#include "physics/joint_hardweld.hpp"
#include "ui/hid/taglistentry.hpp"
#include "util/dynamic.hpp"

namespace clover {
namespace physics {

class Object;

} // physics
namespace nodes {

class PlayerPhysicsEntity {
public:
	using This= PlayerPhysicsEntity;

	void setActive(bool active);
	void reset(	util::RtTransform2d transform,
				bool is_ragdoll,
				animation::ArmaturePose ragdoll_pose,
				game::WorldEntity& we);
	void update(util::Vec2d movement, const animation::ArmaturePose& anim_pose);
	bool canJump() const;
	util::RtTransform2d getTransform() const;
	util::SrtTransform3d getEstimatedTransform() const;
	real64 getMass() const;
	real64 getNormalizedJumpApexDistance() const;
	real64 getNormalizedFeetVelocity() const;
	bool isRagdoll() const;
	animation::ArmaturePose getRagdollPose() const;
	void attachToHand(physics::Object& ob);
	void detachHand();
	void setHandGhostliness(real64 g);
	bool isHandAttached() const;
	real64 popAccumImpulse();
	void applyAcceleration(util::Vec2d a);
	void applyVelocityChange(util::Vec2d i);

private:
	void onGroundContactBegin(const physics::Contact& c);
	void onGroundContactEnd(const physics::Contact& c);
	bool onGroundContactPreSolve(const physics::Contact& c);
	void onBodyContactPostSolve(const physics::PostSolveContact& c);
	real64 wheelOffset();

	int32 touchingGroundCounter= 0;
	util::Vec2d lastContactVelocityOnGround;
	real64 timeFromLastGroundContact= 0.0;
	real64 timeFromLastJump= 0.0;
	real64 facingDir= 0.0;
	real64 smoothFacingDir= 0.0;
	real64 accumImpulse= 0.0;
	util::SrtTransform3d estimatedTransform;
	real64 normalizedJumpApexDistance= 0.0;
	real64 normalizedFeetVelocity= 0.0;

	/// Callbacks will access 'this' at destruction, so these need to be first to destruct
	util::UniquePtr<physics::WheelJoint> wheelJoint;
	util::UniquePtr<physics::RigidObject> bodyObject;
	util::UniquePtr<physics::RigidObject> wheelObject;
	util::UniquePtr<physics::Entity> ragdoll;

	util::Dynamic<physics::ArmatureSuit> physSuit;
	util::Dynamic<physics::HardWeldJoint> handJoint;
};

class PlayerLogicNodeInstance : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual void create() override;
	virtual void update() override;

private:
	void setTagEntry(const util::Str8& s);
	void clearTagEntry();
	void chooseActionState(util::Vec2d pos);
	void tryPickup(util::Vec2d pos);
	physics::Object* findPickableObject(util::Vec2d pos);
	void die();
	void respawn();

private:
	InputSlot<SignalType::String>* playerStringIn;
	InputSlot<SignalType::Vec2>* movementIn;
	InputSlot<SignalType::Trigger>* respawnIn;
	InputSlot<SignalType::Trigger>* killIn;
	InputSlot<SignalType::Vec2>* actionPointIn;
	InputSlot<SignalType::Trigger>* hitEndedIn;
	InputSlot<SignalType::Trigger>* tryPickupIn;
	InputSlot<SignalType::Trigger>* tryDropIn;
	InputSlot<SignalType::Trigger>* tryUseInHandIn;
	InputSlot<SignalType::WeHandle>* weIn;

	OutputSlot<SignalType::Boolean>* deadOut;

	util::RtTransform2d transform;
	real64 accumImpulse;
	real64 health; // full == 1.0
	ui::hid::TagListEntry tagEntry;

	// Physics
	InputSlot<SignalType::Boolean>* activeIn;
	InputSlot<SignalType::RtTransform2>* transformIn;
	InputSlot<SignalType::ArmaturePose>* poseIn;
	
	OutputSlot<SignalType::RtTransform2>* transformOut;
	OutputSlot<SignalType::SrtTransform3>* estimatedTransformOut;
	OutputSlot<SignalType::Real>* normalizedFeetVelocityOut;
	OutputSlot<SignalType::Real>* normalizedJumpApexDistanceOut;
	OutputSlot<SignalType::Trigger>* swingHitOut;
	OutputSlot<SignalType::ArmaturePose>* poseOut;

	PlayerPhysicsEntity physEntity;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::PlayerLogicNodeInstance> {
	static util::Str8 type(){ return "::PlayerLogicNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_PLAYERLOGIC_HPP
