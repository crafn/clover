#ifndef CLOVER_NODES_NODEINSTANCE_MINIONLOGIC_HPP
#define CLOVER_NODES_NODEINSTANCE_MINIONLOGIC_HPP

#include "nodes/nodeinstance.hpp"
#include "build.hpp"
#include "game/worldentity_handle.hpp"
#include "util/unique_ptr.hpp"

namespace clover {
namespace physics { class RigidObject; }
namespace mod {

// I'm lazy
using namespace clover::nodes;

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

DECLARE_NODE(MinionLogicNode);
class MinionLogicNode : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual void create() override;
	void update_novirtual();

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

} // mod
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_MINIONLOGIC_HPP
