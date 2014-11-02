#include "nodeinstance_we_physicsobject.hpp"
#include "collision/baseshape_polygon.hpp"
#include "game/physics.hpp"
#include "game/physicalmaterial.hpp"
#include "physics/fixture_rigid.hpp"
#include "resources/cache.hpp"
#include "util/time.hpp"

namespace clover {
namespace nodes {

void WePhysicsObjectNodeInstance::create(){
	activeInput= addInputSlot<SignalType::Boolean>("active");
	forceInput= addInputSlot<SignalType::RtTransform2>("force");
	transformInput= addInputSlot<SignalType::SrtTransform3>("transform");
	impulseInput= addInputSlot<SignalType::RtTransform2>("impulse");
	shapeInput= addInputSlot<SignalType::Shape>("shape");
	materialInput= addInputSlot<SignalType::String>("material");
	staticInput= addInputSlot<SignalType::Boolean>("static");
	eventInput= addInputSlot<SignalType::Event>("event");
	partialBreakingInput= addInputSlot<SignalType::Boolean>("partialBreaking");
	weInput= addInputSlot<SignalType::WeHandle>("we");
	
	transformOutput= addOutputSlot<SignalType::SrtTransform3>("transform");
	estimatedTransformOutput= addOutputSlot<SignalType::SrtTransform3>("estimatedTransform");
	velocityOutput= addOutputSlot<SignalType::RtTransform2>("velocity");
	impulseOutput= addOutputSlot<SignalType::RtTransform2>("totalImpulse");
	accelerationOutput= addOutputSlot<SignalType::RtTransform2>("acceleration");
	onShapeChangeOutput= addOutputSlot<SignalType::Shape>("onShapeChange");
	onBreakOutput= addOutputSlot<SignalType::Trigger>("onBreak");
	
	setUpdateNeeded();
	
	object.emplace();

	shapeInput->setValueReceived();
	shapeInput->setOnReceiveCallback([&] (){
		recreateObject();
		sendShape();
	});
	
	materialInput->setValueReceived();
	materialInput->setOnReceiveCallback([&] (){
		recreateObject();
	});
	
	activeInput->setOnReceiveCallback([&] (){
		object->setActive(activeInput->get());
		if (activeInput->get())
			setUpdateNeeded();
	});
	
	transformInput->setOnReceiveCallback([&] (){
		object->set3dTransform(transformInput->get());
		// Forward transform if inactive
		if (!activeInput->get() || object->isStatic()){
			transformOutput->send(transformInput->get());
			estimatedTransformOutput->send(transformInput->get());
		}
	});
	
	impulseInput->setOnReceiveCallback([&] (){
		object->applyImpulse(impulseInput->get().translation, object->getPosition());
		object->applyAngularImpulse(impulseInput->get().rotation);
	});
	
	staticInput->setOnReceiveCallback([&] (){
		object->setStatic(staticInput->get());
	});
	
	partialBreakingInput->setOnReceiveCallback([&] (){
		object->setPartiallyBreakable(partialBreakingInput->get());
	});

	weInput->setOnReceiveCallback([&] (){
		game::setOwnerWe(object.ref(), weInput->get().get());
	});
	
	breakListener.listen(*object, [this] (){
		if (object->isFullyBroken())
			onBreakOutput->send();
		else if (onShapeChangeOutput->isAttached() && !object->getFixtures().empty()){
			sendShape();
		}
	});
}

void WePhysicsObjectNodeInstance::update(){
	if (!activeInput->get())
		return;
	
	if (forceInput->get().translation.lengthSqr() > util::epsilon)
		object->applyForce(forceInput->get().translation);
		
	if (util::abs(forceInput->get().rotation) > util::epsilon)
		object->applyTorque(forceInput->get().rotation);

	transformOutput->send(object->get3dTransform());
	estimatedTransformOutput->send(object->getEstimated3dTransform());
	velocityOutput->send(util::RtTransform2d(object->getAngularVelocity(), object->getVelocity()));

	if (impulseOutput->isAttached() || accelerationOutput->isAttached()){
		util::RtTransform2d cur_v;
		cur_v.translation= util::Vec2d(object->getVelocity());
		cur_v.rotation= object->getAngularVelocity();

		util::RtTransform2d d_v;
		d_v.translation= util::Vec2d(cur_v.translation - lastVelocity.translation);
		d_v.rotation= cur_v.rotation - lastVelocity.rotation;

		if (d_v.translation.lengthSqr() > util::epsilon || util::abs(d_v.rotation) > util::epsilon){
			if (accelerationOutput->isAttached()){
				util::RtTransform2d a= d_v;
				a.translation /= util::gGameClock->getDeltaTime();
				a.rotation /= util::gGameClock->getDeltaTime();
				accelerationOutput->send(a);
			}

			if (impulseOutput->isAttached()){
				util::RtTransform2d d_p= d_v;
				d_p.translation *= object->getMass();
				d_p.rotation *= object->getInertia();
				impulseOutput->send(d_p);
			}
		}

		lastVelocity= cur_v;
	}
	
	if (!activeInput->get() || object->isStatic() || object->isFullyBroken())
		setUpdateNeeded(false);
}

void WePhysicsObjectNodeInstance::recreateObject(){
	object->clearFixtures();
	physics::RigidFixtureDef fix_def;
	if (materialInput->get() != "")
		fix_def.setMaterial(resources::gCache->getResource<game::PhysicalMaterial>(materialInput->get()));
	fix_def.setShape(shapeInput->get().get());
	object->add(fix_def);
	object->setCustomData(boost::any(this));
}

void WePhysicsObjectNodeInstance::sendShape(){
	auto shape_ptr= object->getRigidFixtures().front()->getShape();
	if (shape_ptr){
		resources::ResourceRef<collision::Shape> shape;
		// Must make copy because modified shapes are owned by this object
		shape.ref()= *shape_ptr;
		onShapeChangeOutput->send(shape);
	}
}

} // nodes
} // clover