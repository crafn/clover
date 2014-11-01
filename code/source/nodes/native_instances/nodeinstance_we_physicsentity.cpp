#include "collision/baseshape_polygon.hpp"
#include "game/physics.hpp"
#include "game/physicalmaterial.hpp"
#include "nodeinstance_we_physicsentity.hpp"
#include "physics/fixture_rigid.hpp"
#include "resources/cache.hpp"
#include "util/time.hpp"

namespace clover {
namespace nodes {

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
