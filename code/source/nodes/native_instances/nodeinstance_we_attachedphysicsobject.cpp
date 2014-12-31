#include "batchpriorities.hpp"
#include "collision/query.hpp"
#include "game/physics.hpp"
#include "nodeinstance_we_attachedphysicsobject.hpp"

namespace clover {
namespace nodes {

CompNode* WeAttachedPhysicsObjectNodeInstance::compNode()
{
	auto n= new CompNode{};
	n->addInputSlot("active", SignalType::Boolean, true);
	n->addInputSlot("transform", SignalType::SrtTransform3);
	n->addInputSlot("anchor", SignalType::Vec2);
	n->addInputSlot("we", SignalType::WeHandle);

	n->addOutputSlot("transform", SignalType::SrtTransform3);
	n->addOutputSlot("estimatedTransform", SignalType::SrtTransform3);
	n->addOutputSlot("detached", SignalType::Trigger);

	n->setBatched(true);
	n->setBatchPriority(nodeBatchPriority_physicsEntity);
	return n;
}

void WeAttachedPhysicsObjectNodeInstance::create()
{
	activeIn= addInputSlot<SignalType::Boolean>("active");
	transformIn= addInputSlot<SignalType::SrtTransform3>("transform");
	anchorIn= addInputSlot<SignalType::Vec2>("anchor");
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

	anchorIn->setOnReceiveCallback([&] ()
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
	detachListener.clear();
	util::Vec2d p= anchorIn->get();

	auto obj_t= transformIn->get();
	physics::RigidObjectDef def;
	def.setPosition(obj_t.translation.xy());
	object= util::makeUniquePtr<physics::RigidObject>(def);
	object->set3dTransform(obj_t);
	game::setOwnerWe(object.ref(), weInput->get().get());

	util::DynArray<physics::Fixture*> fixtures;
	collision::Query::fixture.point(p, [&] (physics::Fixture& t){
		fixtures.pushBack(&t);
		return true;
	});

	bool attached= false;
	for (SizeType i= 0; i < fixtures.size(); ++i) {
		/// @todo Input slot controlling attaching
		if (!fixtures[i]->getObject().isStatic())
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
