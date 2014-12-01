#include "collision/query.hpp"
#include "game/physics.hpp"
#include "nodeinstance_we_attachedphysicsobject.hpp"

namespace clover {
namespace nodes {

void WeAttachedPhysicsObjectNodeInstance::create()
{
	activeIn= addInputSlot<SignalType::Boolean>("active");
	transformIn= addInputSlot<SignalType::SrtTransform3>("transform");
	weInput= addInputSlot<SignalType::WeHandle>("we");

	transformOut= addOutputSlot<SignalType::SrtTransform3>("transform");
	estimatedOut= addOutputSlot<SignalType::SrtTransform3>("estimatedTransform");
	detachedOut= addOutputSlot<SignalType::Trigger>("detached");

	setUpdateNeeded(true);

	activeIn->setOnReceiveCallback([&] ()
	{
		if (object)
			object->setActive(activeIn->get());
		setUpdateNeeded(true);
	});

	transformIn->setOnReceiveCallback([&] ()
	{
		attach();
		setUpdateNeeded(true);
	});

	weInput->setOnReceiveCallback([&] (){
		if (object)
			game::setOwnerWe(object.ref(), weInput->get().get());
	});
}

void WeAttachedPhysicsObjectNodeInstance::update()
{
	if (!object) {
		setUpdateNeeded(false);
		return;
	}

	transformOut->send(object->get3dTransform());
	estimatedOut->send(object->getEstimated3dTransform());

	if (!objectCanMove())
		setUpdateNeeded(false);
}

void WeAttachedPhysicsObjectNodeInstance::attach()
{
	util::Vec2d p= transformIn->get().translation.xy();

	physics::RigidObjectDef def;
	def.setPosition(p);
	object= util::makeUniquePtr<physics::RigidObject>(def);
	object->set3dTransform(transformIn->get());
	game::setOwnerWe(object.ref(), weInput->get().get());

	util::DynArray<physics::Fixture*> fixtures;
	collision::Query::fixture.point(p, [&] (physics::Fixture& t){
		fixtures.pushBack(&t);
		return true;
	});

	bool attached= false;
	for (SizeType i= 0; i < fixtures.size(); ++i) {
		/// @todo Input slot controlling attaching
		if (!fixtures[0]->getObject().isStatic())
			continue;
		detachListener.clear();
		joint.get().attach(
				object.ref(), 
				fixtures[i]->getObject(),
				p,
				p);

		detachListener.listen(joint.get(), [this]() { onDetach(); });
		attached= true;
	}
	if (!attached) {
		onDetach();
	}
}

void WeAttachedPhysicsObjectNodeInstance::onDetach()
{
	detachedOut->send();
	detachListener.clear();
}

bool WeAttachedPhysicsObjectNodeInstance::objectCanMove() const
{
	if (!object)
		return false;
	if (!object->isActive() || object->isStatic())
		return false;
	if (joint->isAttached() && joint->getObject(1).isStatic())
		return false;
	
	return true;
}

} // nodes
} // clover
