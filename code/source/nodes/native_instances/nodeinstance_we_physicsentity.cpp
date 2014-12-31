#include "batchpriorities.hpp"
#include "collision/baseshape_polygon.hpp"
#include "game/physics.hpp"
#include "game/physicalmaterial.hpp"
#include "nodeinstance_we_physicsentity.hpp"
#include "physics/fixture_rigid.hpp"
#include "resources/cache.hpp"
#include "util/time.hpp"

namespace clover {
namespace nodes {

CompNode* WePhysicsEntityNodeInstance::compNode()
{
	auto n= new CompNode{};
	n->addInputSlot("active", SignalType::Boolean, true);
	n->addInputSlot("transform", SignalType::SrtTransform3);
	n->addInputSlot("entityDef", SignalType::String, util::Str8(""));
	n->addInputSlot("we", SignalType::WeHandle);

	n->addOutputSlot("transform", SignalType::SrtTransform3);
	n->addOutputSlot("estimatedTransform", SignalType::SrtTransform3);
	n->addOutputSlot("pose", SignalType::ArmaturePose);

	n->setBatched(true);
	n->setBatchPriority(nodeBatchPriority_physicsEntity);
	return n;
}

void WePhysicsEntityNodeInstance::create(){
	activeInput= addInputSlot<SignalType::Boolean>("active");
	transformInput= addInputSlot<SignalType::SrtTransform3>("transform");
	entityDefInput= addInputSlot<SignalType::String>("entityDef");
	weInput= addInputSlot<SignalType::WeHandle>("we");

	transformOutput= addOutputSlot<SignalType::SrtTransform3>("transform");
	estimatedTransformOutput= addOutputSlot<SignalType::SrtTransform3>("estimatedTransform");
	poseOutput= addOutputSlot<SignalType::ArmaturePose>("pose");

	setUpdateNeeded();

	activeInput->setOnReceiveCallback([&] (){
		entity.setActive(activeInput->get());
		if (activeInput->get())
			setUpdateNeeded();
	});

	transformInput->setOnReceiveCallback([&] (){
		recreateEntity();
		// Forward transform if inactive
		if (!activeInput->get()){
			transformOutput->send(transformInput->get());
			estimatedTransformOutput->send(transformInput->get());
		}
	});

	weInput->setOnReceiveCallback([&] (){
		game::setOwnerWe(entity, weInput->get().get());
	});
}

void WePhysicsEntityNodeInstance::update(){
	if (!activeInput->get())
		return;

	transformOutput->send(entity.getTransform());
	estimatedTransformOutput->send(entity.getEstimatedTransform());
	if (entity.hasArmature())
		poseOutput->send(entity.getEstimatedPose());

	if (!activeInput->get())
		setUpdateNeeded(false);
}

void WePhysicsEntityNodeInstance::recreateEntity(){
	entity.set(transformInput->get(), entityDefInput->get());
	game::setOwnerWe(entity, weInput->get().get());
}

} // nodes
} // clover
