#ifndef CLOVER_NODES_NODEINSTANCE_MINIONLOGIC_HPP
#define CLOVER_NODES_NODEINSTANCE_MINIONLOGIC_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"
#include "game/worldentity_handle.hpp"

namespace clover {
namespace physics {

class RigidObject;

} // physics
namespace nodes {

class MinionPhysicsEntity {
public:
	void setActive(bool active);
	void reset(util::RtTransform2d t);
	void setTarget(util::Vec2d pos);
	void setAwake(bool b);
	real64 targetDistance();
	void setWe(game::WeHandle h);
	void update();

	util::RtTransform2d getTransform() const;
	util::RtTransform2d getEstimatedTransform() const;

private:
	util::UniquePtr<physics::RigidObject> bodyObject;
	util::Vec2d target;
	bool awake= false;
};

class MinionLogicNodeInstance : public NodeInstance {
public:
	static CompNode* compNode();

	virtual void create() override;
	virtual void update() override;

private:
	InputSlot<SignalType::Boolean>* activeIn;
	InputSlot<SignalType::RtTransform2>* transformIn;
	InputSlot<SignalType::Trigger>* wakeIn;
	InputSlot<SignalType::WeHandle>* weIn;

	OutputSlot<SignalType::RtTransform2>* transformOut;
	OutputSlot<SignalType::SrtTransform3>* estimatedTransformOut;

	util::RtTransform2d transform;
	game::WeHandle targetWe;
	real64 phase= 0.0;
	MinionPhysicsEntity physEntity;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::MinionLogicNodeInstance> {
	static util::Str8 type(){ return "::MinionLogicNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_MINIONLOGIC_HPP
