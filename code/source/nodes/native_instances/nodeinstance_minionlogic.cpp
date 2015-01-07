#include "collision/query.hpp"
#include "game/physics.hpp"
#include "game/world_mgr.hpp"
#include "game/worldentity.hpp"
#include "global/env.hpp"
#include "nodeinstance_minionlogic.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* MinionLogicNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->addInputSlot("active", SignalType::Boolean, false);
	n->addInputSlot("transform", SignalType::RtTransform2);
	n->addInputSlot("wake", SignalType::Trigger);
	n->addInputSlot("we", SignalType::WeHandle);
	n->addOutputSlot("transform", SignalType::RtTransform2);
	n->addOutputSlot("estimatedTransform", SignalType::SrtTransform3);
	return n;
}

void MinionPhysicsEntity::setActive(bool active)
{
	if (bodyObject)
		bodyObject->setActive(active);
}

void MinionPhysicsEntity::reset(util::RtTransform2d t)
{
	bodyObject= 
			util::makeUniquePtr<physics::RigidObject>(
					physics::RigidObjectDef{t.translation});
	bodyObject->setTransform(t);
	bodyObject->setGhostliness(0.0);
	bodyObject->setGravityScale(0.2);

	physics::RigidFixtureDef fix_def;
	fix_def.setShape("minion_body");
	fix_def.setMaterial("minion_body");
	bodyObject->add(fix_def);
}

void MinionPhysicsEntity::setTarget(util::Vec2d pos)
{ target= pos; }

void MinionPhysicsEntity::setAwake(bool b)
{ awake= b; }

real64 MinionPhysicsEntity::targetDistance()
{ return (target - bodyObject->getPosition()).length(); }

void MinionPhysicsEntity::setWe(game::WeHandle h)
{ setOwnerWe(bodyObject.ref(), h.get()); }

void MinionPhysicsEntity::update()
{
	const real64 max_accel= 15.0;
	const real64 max_alpha= 10;

	util::Vec2d pos= bodyObject->getTransform().translation;
	real64 rot= bodyObject->getTransform().rotation;
	real64 mass= bodyObject->getMass();
	real64 inertia= bodyObject->getInertia();
	util::Vec2d v= bodyObject->getVelocity();
	real64 w= bodyObject->getAngularVelocity();
	util::Vec2d dif= target - pos;

	util::Vec2d a;
	real64 alpha= 0;
	if (awake) {
		// Flying
		a= dif*20;
		if (a.lengthSqr() > max_accel*max_accel)
			a= a.normalized()*max_accel;	
		bodyObject->applyForce(a*mass);

		// Facing dir
		real64 target_rot= -a.x/max_accel*0.8;
		alpha= (target_rot - rot)*20;
		if (alpha > max_alpha)
			alpha= max_alpha;
		else if (alpha < -max_alpha)
			alpha= -max_alpha;
	}

	// Drag
	a += -v*3;
	alpha += -w*8;

	if (a.lengthSqr() > 0.001 || std::abs(alpha) > 0.001) {
		bodyObject->applyForce(a*mass);
		bodyObject->applyTorque(alpha*inertia);
	}
}

util::RtTransform2d MinionPhysicsEntity::getTransform() const
{ return bodyObject->getTransform(); }

util::RtTransform2d MinionPhysicsEntity::getEstimatedTransform() const
{ return bodyObject->getTransform(); }

void MinionLogicNodeInstance::create()
{
	activeIn= addInputSlot<SignalType::Boolean>("active");
	transformIn= addInputSlot<SignalType::RtTransform2>("transform");
	wakeIn= addInputSlot<SignalType::Trigger>("wake");
	weIn= addInputSlot<SignalType::WeHandle>("we");

	transformOut= addOutputSlot<SignalType::RtTransform2>("transform");
	estimatedTransformOut= addOutputSlot<SignalType::SrtTransform3>("estimatedTransform");

	transformIn->setValueReceived();
	setUpdateNeeded(true);

	transformIn->setOnReceiveCallback([&] ()
	{
		transform= transformIn->get();
		physEntity.reset(transform);
		physEntity.setTarget(transform.translation); // Test
	});

	activeIn->setOnReceiveCallback([&] ()
	{
		physEntity.setActive(activeIn->get());
	});

	wakeIn->setOnReceiveCallback([&] ()
	{
		if (!targetWe.get()) {
			/// @todo Skip if triggered again in a short period
			const real64 max_dist= 4;

			util::DynArray<physics::Fixture*> fixtures;
			collision::Query::fixture.potentialRect(transform.translation, util::Vec2d(max_dist),
			[&] (physics::Fixture& t)
			{ fixtures.pushBack(&t); return true; });

			for (SizeType i= 0; i < fixtures.size(); ++i){
				physics::Object& obj= fixtures[i]->getObject();
				if (obj.isStatic())
					continue;

				game::WorldEntity* we= game::getOwnerWe(obj);
				if (!we)
					continue;

				if (we->getTypeName() != "testCharacter")
					continue;

				targetWe.setId(we->getId());
				phase= util::Rand::continuous(0.0, 100.0);
				physEntity.setAwake(true);
				break;
			}
		}
	});

	weIn->setOnReceiveCallback([&] ()
	{ physEntity.setWe(weIn->get()); });
}

void MinionLogicNodeInstance::update()
{
	real64 dt= global::g_env.worldMgr->getDeltaTime();
	phase += dt;

	if (targetWe.get()) {
		util::Vec2d pos= targetWe->getPosition() + util::Vec2d{0, 2};
		pos.x += std::sin(phase)*1.3;
		pos.y += std::cos(phase*0.7)*0.8;

		physEntity.setTarget(pos);

		if (physEntity.targetDistance() > 10.0) {
			physEntity.setAwake(false);
			targetWe.reset();
		}
	}

	physEntity.update();
	transform= physEntity.getTransform();

	transformOut->send(transform);

	util::RtTransform2d estimated_t2d= physEntity.getEstimatedTransform();
	util::SrtTransform3d estimated_t;
	estimated_t.translation=
		util::Vec3d{	estimated_t2d.translation.x,
						estimated_t2d.translation.y,
						0.0};
	estimated_t.rotation=
		util::Quatd::byRotationAxis(util::Vec3d{0, 0, 1}, estimated_t2d.rotation);
	estimatedTransformOut->send(estimated_t);
}

} // nodes
} // clover
