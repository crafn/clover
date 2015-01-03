#include "collision/baseshape_circle.hpp"
#include "collision/query.hpp"
#include "game/physics.hpp"
#include "game/physicalmaterial.hpp"
#include "game/world_mgr.hpp"
#include "global/env.hpp"
#include "nodeinstance_playerlogic.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* PlayerLogicNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->addInputSlot("player", SignalType::String, util::Str8("player0"));
	n->addInputSlot("movement", SignalType::Vec2);
	n->addInputSlot("respawn", SignalType::Trigger);
	n->addInputSlot("kill", SignalType::Trigger);
	n->addInputSlot("actionPoint", SignalType::Vec2);
	n->addInputSlot("hitEnded", SignalType::Trigger);
	n->addInputSlot("tryPickup", SignalType::Trigger);
	n->addInputSlot("tryDrop", SignalType::Trigger);
	n->addInputSlot("tryUseInHand", SignalType::Trigger);
	n->addInputSlot("we", SignalType::WeHandle);

	n->addOutputSlot("dead", SignalType::Boolean);

	// Physics

	n->addInputSlot("active", SignalType::Boolean, false);
	n->addInputSlot("transform", SignalType::RtTransform2);
	n->addInputSlot("pose", SignalType::ArmaturePose);
	
	n->addOutputSlot("transform", SignalType::RtTransform2);
	n->addOutputSlot("estimatedTransform", SignalType::SrtTransform3);
	n->addOutputSlot("normalizedFeetVelocity", SignalType::Real);
	n->addOutputSlot("normalizedJumpApexDistance", SignalType::Real);
	n->addOutputSlot("swingHit", SignalType::Trigger);
	n->addOutputSlot("pose", SignalType::ArmaturePose);

	return n;
}

void PlayerPhysicsEntity::setActive(bool active)
{
	if (bodyObject)
		bodyObject->setActive(active);
	if (wheelObject)
		wheelObject->setActive(active);
}

void PlayerPhysicsEntity::reset(util::RtTransform2d transform,
								bool is_ragdoll,
								animation::ArmaturePose ragdoll_pose,
								game::WorldEntity& we)
{
	using namespace std::placeholders;

	detachHand();
	if (!is_ragdoll){
		ragdoll.reset();

		// Wheel
		physics::RigidFixtureDef fix_def;
		fix_def.setShape("player_collision_motor");
		fix_def.setMaterial("player_collision_motor");

		util::RtTransform2d wheel_t= transform;
		wheel_t.translation.y += wheelOffset();
		wheelObject=
			util::makeUniquePtr<physics::RigidObject>(
					physics::RigidObjectDef{wheel_t.translation});
		game::setOwnerWe(wheelObject.ref(), &we);
		wheelObject->setTransform(wheel_t);
		wheelObject->add(fix_def);

		// Body
		bodyObject=
			util::makeUniquePtr<physics::RigidObject>(
					physics::RigidObjectDef{transform.translation});
		bodyObject->setPosition(transform.translation);
		bodyObject->setFixedRotation(true);
		setOwnerWe(bodyObject.ref(), &we);

		fix_def.setShape("player_collision");
		fix_def.setMaterial("player_collision");
		physics::RigidFixture& body_fix= bodyObject->add(fix_def);
		body_fix.getCallbacks().onPostSolveContact= std::bind(&This::onBodyContactPostSolve, this, _1);

		fix_def.setShape("player_sensor_ground");
		fix_def.setAsSensor(true);
		physics::RigidFixture& ground_fix= bodyObject->add(fix_def);

		ground_fix.getCallbacks().onBeginContact= std::bind(&This::onGroundContactBegin, this, _1);
		ground_fix.getCallbacks().onEndContact= std::bind(&This::onGroundContactEnd, this, _1);
		ground_fix.getCallbacks().onPreSolveContact= std::bind(&This::onGroundContactPreSolve, this, _1);

		// Wheel joint
		wheelJoint.emplace();
		wheelJoint->attach(bodyObject.ref(), wheelObject.ref(), util::Vec2d{0.0, 1.0});
		wheelJoint->setSpringFrequency(4.0);
		wheelJoint->setSpringDamping(1.0);
		wheelJoint->enableMotor(true);
		wheelJoint->setMaxMotorTorque(60.0);
	} else {
		wheelObject->setActive(false);
		bodyObject->setActive(false);
		
		ragdoll.emplace();
		ragdoll->setAsRagdoll(	bodyObject->getTransform(),
								0.08,
								global::g_env.resCache->getResource<game::PhysicalMaterial>("player_ragdoll"),
								ragdoll_pose);
	}
}

void PlayerPhysicsEntity::update(util::Vec2d movement, const animation::ArmaturePose& anim_pose)
{
	real64 dt= global::g_env.worldMgr->getDeltaTime();
	// Smooth changing of direction
	real64 blend_factor= std::pow(util::euler, -14.0*dt);
	smoothFacingDir= smoothFacingDir*blend_factor + facingDir*(1.0 - blend_factor);
	smoothFacingDir= util::limited(smoothFacingDir, -1.0, 1.0);

	// Send transform
	if (!ragdoll) {
		util::RtTransform2d t_2d= bodyObject->getEstimatedTransform();
		util::SrtTransform3d t_3d;
		t_3d.translation= util::Vec3d{t_2d.translation.x, t_2d.translation.y, 0.0};
		t_3d.rotation=
			util::Quatd::byRotationAxis(
					util::Vec3d{0.0, 1.0, 0.0},
					(smoothFacingDir - 1.0)*0.5*util::pi);
		estimatedTransform= t_3d;
	} else {
		util::Vec2d pos_2d= ragdoll->getEstimatedPosition();
		util::SrtTransform3d t;
		t.translation.x= pos_2d.x;
		t.translation.y= pos_2d.y;
		estimatedTransform= t;
	}

	// Distance from apex
	real64 apex_dist= 1.0;
	if (timeFromLastJump < 3.0 && timeFromLastGroundContact + 0.1 > timeFromLastJump) {
		real64 vel_dif_y= std::abs(bodyObject->getEstimatedVelocity().translation.y*0.1 - lastContactVelocityOnGround.y);
		apex_dist= util::limited(vel_dif_y, 0.0, 1.0);
	}
	normalizedJumpApexDistance= apex_dist;

	// Moving sideways on foot
	const real64 max_omega= 18.0;
	real64 omega= -max_omega*movement.x;
	wheelJoint->setTargetAngularVelocity(omega);
	if (movement.x > 0.0)
		facingDir= 1.0;
	else if (movement.x < 0.0)
		facingDir= -1.0;

	normalizedFeetVelocity= std::abs(wheelObject->getEstimatedVelocity().rotation/max_omega);

	if (touchingGroundCounter > 0) { // On ground
		timeFromLastGroundContact= 0.0;
	} else { // On air
		timeFromLastGroundContact += dt;

		// Moving sideways
		const real64 max_vel_dif= 8.0; // Controls how fast player can move relative to jumping v.x
		const real64 a_max= 30.0;
		const real64 a_slope_factor= 3.0;

		real64 v= bodyObject->getVelocity().x;
		real64 target_v= v + movement.x*max_vel_dif;
		target_v= util::limited(target_v,
								lastContactVelocityOnGround.x - max_vel_dif,
								lastContactVelocityOnGround.x + max_vel_dif);

		real64 v_dif= target_v - v;
		real64 a= a_max*(1.0 - std::pow(a_slope_factor, -v_dif*v_dif));

		if (v_dif < 0.0)
			a= -a;

		applyAcceleration(util::Vec2d{a, 0.0});

		// Friction
		applyAcceleration(util::Vec2d{lastContactVelocityOnGround.x - v, 0.0});
	}

	timeFromLastJump += dt;

	// Start jump
	if (movement.y >= 1.0 && canJump()){
		util::Vec2d vel_delta{0.0, 10.0};
		// Jump velocity is relative to the ground
		vel_delta.y += lastContactVelocityOnGround.y - bodyObject.ref().getVelocity().y;
		applyVelocityChange(vel_delta);
		timeFromLastJump= 0.0;
	}

	// Jump maintaining
	const real64 jump_maintain_time= 0.2;
	if (timeFromLastJump < jump_maintain_time){
		real64 a= 0.0;
		if (movement.y > 0.0)
			a= 5.0;
		else
			a= -20.0;

		applyAcceleration(util::Vec2d{0.0, a});
	}
	
	if (!ragdoll)
		physSuit->update(getEstimatedTransform(), anim_pose);
	else
		physSuit->update(getEstimatedTransform(), getRagdollPose());
}

bool PlayerPhysicsEntity::canJump() const
{ return timeFromLastGroundContact < 0.1 && timeFromLastJump > 0.2; } 

util::RtTransform2d PlayerPhysicsEntity::getTransform() const
{ return bodyObject->getTransform(); }

util::SrtTransform3d PlayerPhysicsEntity::getEstimatedTransform() const
{ return estimatedTransform; }

real64 PlayerPhysicsEntity::getMass() const
{ return wheelObject.ref().getMass() + bodyObject.ref().getMass(); }

real64 PlayerPhysicsEntity::getNormalizedJumpApexDistance() const
{ return normalizedJumpApexDistance; }

real64 PlayerPhysicsEntity::getNormalizedFeetVelocity() const
{ return normalizedFeetVelocity; }

bool PlayerPhysicsEntity::isRagdoll() const
{ return ragdoll.get() != nullptr; }

animation::ArmaturePose PlayerPhysicsEntity::getRagdollPose() const
{ return ragdoll->getPose(); }

void PlayerPhysicsEntity::attachToHand(physics::Object& ob)
{
	detachHand();

	util::DynArray<physics::Object*> connected= ob.getGraphObjects();
	for (SizeType i= 0; i < connected.size(); ++i){
		physics::Object& c= *NONULL(connected[i]);
		bodyObject->addIgnoreObject(c);
		wheelObject->addIgnoreObject(c);
	}

	physics::RigidObject& hand= physSuit->get("right_hand");
	physics::Entity* entity= ob.getEntity();
	if (entity) {
		physics::Object& root= entity->getRootObject("handle");
		util::SrtTransform3d from_ob_to_hand=
			entity->getJointTransform("handle")*root.get3dTransform().inversed();
		root.set3dTransform(from_ob_to_hand.inversed()*hand.get3dTransform());
		handJoint->attach(root, hand);
	} else {
		ob.set3dTransform(hand.get3dTransform());
		handJoint->attach(ob, hand);
	}
}

void PlayerPhysicsEntity::detachHand()
{
	if (!handJoint->isAttached())
		return;

	setHandGhostliness(0.0);

	physics::Object& attached= handJoint->getObject(1);

	util::DynArray<physics::Object*> connected= attached.getGraphObjects();
	for (SizeType i= 0; i < connected.size(); ++i){
		physics::Object& c= *NONULL(connected[i]);
		bodyObject->removeIgnoreObject(c);
		wheelObject->removeIgnoreObject(c);
	}

	handJoint->detach();
}

void PlayerPhysicsEntity::setHandGhostliness(real64 g)
{
	if (!handJoint->isAttached())
		return;

	physics::Object& attached= handJoint->getObject(1);

	util::DynArray<physics::Object*> connected= attached.getGraphObjects();
	for (SizeType i= 0; i < connected.size(); ++i){
		physics::Object& c= *NONULL(connected[i]);
		c.setGhostliness(g);
	}
}

bool PlayerPhysicsEntity::isHandAttached() const
{ return handJoint->isAttached(); }

real64 PlayerPhysicsEntity::popAccumImpulse()
{
	real64 ret= accumImpulse;
	accumImpulse= 0.0;
	return ret;
}

void PlayerPhysicsEntity::applyAcceleration(util::Vec2d a)
{
	wheelObject->applyForce(a*wheelObject->getMass());
	bodyObject->applyForce(a*bodyObject->getMass());
}

void PlayerPhysicsEntity::applyVelocityChange(util::Vec2d i)
{
	wheelObject->applyImpulse(i*wheelObject->getMass());
	bodyObject->applyImpulse(i*bodyObject->getMass());
}

void PlayerPhysicsEntity::onGroundContactBegin(const physics::Contact& c)
{ ++touchingGroundCounter; }

void PlayerPhysicsEntity::onGroundContactEnd(const physics::Contact& c)
{ --touchingGroundCounter; }

bool PlayerPhysicsEntity::onGroundContactPreSolve(const physics::Contact& c)
{
		/// @todo Velocity should be updated every frame, not only when contact is formed
		lastContactVelocityOnGround=
			NONULL(c.getSide(1).object)->getVelocity(
					NONULL(c.getSide(0).object)->getPosition());
		return true;
}

void PlayerPhysicsEntity::onBodyContactPostSolve(const physics::PostSolveContact& c)
{ accumImpulse += c.getSide(0).totalImpulse.length(); }

real64 PlayerPhysicsEntity::wheelOffset()
{ return -0.65; }

void PlayerLogicNodeInstance::create()
{
	playerStringIn= addInputSlot<SignalType::String>("player");
	movementIn= addInputSlot<SignalType::Vec2>("movement");
	respawnIn= addInputSlot<SignalType::Trigger>("respawn");
	killIn= addInputSlot<SignalType::Trigger>("kill");
	actionPointIn= addInputSlot<SignalType::Vec2>("actionPoint");
	hitEndedIn= addInputSlot<SignalType::Trigger>("hitEnded");
	poseIn= addInputSlot<SignalType::ArmaturePose>("pose");
	tryPickupIn= addInputSlot<SignalType::Trigger>("tryPickup");
	tryDropIn= addInputSlot<SignalType::Trigger>("tryDrop");
	tryUseInHandIn= addInputSlot<SignalType::Trigger>("tryUseInHand");
	weIn= addInputSlot<SignalType::WeHandle>("we");

	deadOut= addOutputSlot<SignalType::Boolean>("dead");

	accumImpulse= 0.0;
	health= 1.0;

	// Physics
	transformIn= addInputSlot<SignalType::RtTransform2>("transform");
	activeIn= addInputSlot<SignalType::Boolean>("active");

	transformOut= addOutputSlot<SignalType::RtTransform2>("transform");
	estimatedTransformOut= addOutputSlot<SignalType::SrtTransform3>("estimatedTransform");
	normalizedFeetVelocityOut= addOutputSlot<SignalType::Real>("normalizedFeetVelocity");
	normalizedJumpApexDistanceOut= addOutputSlot<SignalType::Real>("normalizedJumpApexDistance");
	swingHitOut= addOutputSlot<SignalType::Trigger>("swingHit");
	poseOut= addOutputSlot<SignalType::ArmaturePose>("pose");

	transformIn->setValueReceived();

	transformIn->setOnReceiveCallback([&] ()
	{
		transform= transformIn->get();
		respawn();
	});

	respawnIn->setOnReceiveCallback([&] ()
	{ respawn(); });

	killIn->setOnReceiveCallback([&] ()
	{ die(); });

	activeIn->setOnReceiveCallback([&] ()
	{ physEntity.setActive(activeIn->get()); });

	hitEndedIn->setOnReceiveCallback([&] ()
	{ physEntity.setHandGhostliness(1.0); });

	tryPickupIn->setOnReceiveCallback([&] ()
	{ tryPickup(actionPointIn->get()); });

	tryDropIn->setOnReceiveCallback([&] ()
	{ physEntity.detachHand(); });

	tryUseInHandIn->setOnReceiveCallback([&] ()
	{
		swingHitOut->send();
		physEntity.setHandGhostliness(0.2);
	});

	setUpdateNeeded(true);
}

void PlayerLogicNodeInstance::update()
{
	real64 dt= global::g_env.worldMgr->getDeltaTime();

	physEntity.update(movementIn->get(), poseIn->get());
	transform= physEntity.getTransform();

	accumImpulse += physEntity.popAccumImpulse();
	if (accumImpulse > 150.0){
		accumImpulse= 0.0;
		die();
	}
	accumImpulse *= pow(2, -1.0*dt);

	chooseActionState(actionPointIn->get());

	if (physEntity.isRagdoll())
		poseOut->send(physEntity.getRagdollPose());
	else
		poseOut->send(poseIn->get());

	transformOut->send(transform);
	
	util::SrtTransform3d estimated_t= physEntity.getEstimatedTransform();
	estimated_t.translation.z -= 0.05; // Keeps legs behind grass
	estimatedTransformOut->send(estimated_t);

	normalizedFeetVelocityOut->send(physEntity.getNormalizedFeetVelocity());
	normalizedJumpApexDistanceOut->send(physEntity.getNormalizedJumpApexDistance());
}

void PlayerLogicNodeInstance::setTagEntry(const util::Str8& s)
{
	tagEntry= ui::hid::TagListEntry(
		playerStringIn->get(),
		"charCtrl_" + s,
		"charCtrl");
}

void PlayerLogicNodeInstance::clearTagEntry()
{
	tagEntry= util::optionalNone;
}

void PlayerLogicNodeInstance::chooseActionState(util::Vec2d pos)
{
	if (physEntity.isHandAttached()) {
		setTagEntry("inHand");
	} else {
		physics::Object* obj= findPickableObject(pos);
		if (obj) {
			game::WorldEntity* we= game::getOwnerWe(*obj);

			if (!we) {
				clearTagEntry();
			} else {
				setTagEntry("hover_pickup");

				NodeEvent e("highlight");
				e.addReceiver(*we);
				e.send();
			}
		}
	}
}

void PlayerLogicNodeInstance::tryPickup(util::Vec2d pos)
{
	physics::Object* obj= findPickableObject(pos);
	if (obj) {
		physEntity.attachToHand(*obj);
		physEntity.setHandGhostliness(1.0);
	}
}

physics::Object* PlayerLogicNodeInstance::findPickableObject(util::Vec2d pos)
{
	real32 max_dist= 0.5;

	// Test shape at `pos`
	collision::CircleBaseShape c;
	c.setRadius(0.0);
	collision::Shape test_shp;
	test_shp.add(c);
	util::RtTransform2d test_shp_t(0.0, pos);

	real32 closest_dist= max_dist;
	physics::Object* closest_obj= nullptr;
	util::DynArray<physics::Fixture*> fixtures;
	collision::Query::fixture.potentialRect(pos, util::Vec2d(max_dist),
	[&] (physics::Fixture& t) {
		fixtures.pushBack(&t);
		return true;
	});
	for (SizeType i= 0; i < fixtures.size(); ++i) {
		physics::Object& obj= fixtures[i]->getObject();

		if (obj.getInvMass() < 1.0/100.0)
			continue;

		const collision::Shape& fix_shp= *NONULL(fixtures[i]->getShape());
		const collision::Shape& shp= test_shp;
		real32 dist= fix_shp.distance(shp, test_shp_t, obj.getTransform());
		if (dist < closest_dist){
			closest_dist= dist;
			closest_obj= &obj;
		}
	}
	return closest_obj;
}

void PlayerLogicNodeInstance::die()
{
	health= 0.0;
	physEntity.reset(transform, true, poseIn->get(), *NONULL(weIn->get().get()));
	deadOut->send(true);
}

void PlayerLogicNodeInstance::respawn()
{
	health= 1.0;
	physEntity.reset(transform, false, poseIn->get(), *NONULL(weIn->get().get()));
	deadOut->send(false);
}

} // nodes
} // clover
