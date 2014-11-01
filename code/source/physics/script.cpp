#include "script.hpp"
#include "animation/armature.hpp"
#include "animation/armaturepose.hpp"
#include "collision/baseshape.hpp"
#include "collision/baseshape_circle.hpp"
#include "collision/baseshape_polygon.hpp"
#include "collision/query.hpp"
#include "collision/shape.hpp"
#include "collision/traceable.hpp"
#include "entity.hpp"
#include "object_rigid.hpp"
#include "physics/armaturesuit.hpp"
#include "physics/joints.hpp"
#include "script/script_mgr.hpp"
#include "util/shared_ptr.hpp"
#include "util/unique_ptr.hpp"

namespace clover {
namespace physics {

//
// Script interface
//

util::DynArray<collision::Traceable*> pointQuery(util::Vec2d pos){
	util::DynArray<collision::Traceable*> queried;
	collision::Query::point(pos, [&] (collision::Traceable& t){
		queried.pushBack(&t);
		return true;
	});
	return queried;
}

util::DynArray<collision::Traceable*> potentialRectQuery(util::Vec2d pos, util::Vec2d rad){
	util::DynArray<collision::Traceable*> queried;
	collision::Query::potentialRect(pos, rad, [&] (collision::Traceable& t){
		queried.pushBack(&t);
		return true;
	});
	return queried;
}


util::DynArray<Fixture*> fixturePointQuery(util::Vec2d pos){
	util::DynArray<Fixture*> queried;
	collision::Query::fixture.point(pos, [&] (Fixture& t){
		queried.pushBack(&t);
		return true;
	});
	return queried;
}

util::DynArray<Fixture*> fixturePotentialRectQuery(util::Vec2d pos, util::Vec2d rad){
	util::DynArray<Fixture*> queried;
	collision::Query::fixture.potentialRect(pos, rad, [&] (Fixture& t){
		queried.pushBack(&t);
		return true;
	});
	return queried;
}

template <typename T>
void registerObjectMethods(){
	auto& s= *script::gScriptMgr;
	
	s.registerMethod<void (T::*)(util::Vec2d, util::Vec2d)>(&T::applyForce, "applyForce");
	s.registerMethod<void (T::*)(util::Vec2d)>(&T::applyForce, "applyForce");
	s.registerMethod<void (T::*)(util::Vec2d, util::Vec2d)>(&T::applyImpulse, "applyImpulse");
	s.registerMethod<void (T::*)(util::Vec2d)>(&T::applyImpulse, "applyImpulse");
	s.registerMethod<T>(&T::applyTorque, "applyTorque");
	
	s.registerMethod<T>(&T::getTransform, "getTransform");
	s.registerMethod<T>(&T::getPosition, "getPosition");
	s.registerMethod<T>(&T::getRotation, "getRotation");
	s.registerMethod<T>(&T::getEstimatedTransform, "getEstimatedTransform");
	
	s.registerMethod<T>(&T::setTransform, "setTransform");
	s.registerMethod<T>(&T::setPosition, "setPosition");
	s.registerMethod<T>(&T::setRotation, "setRotation");
	
	s.registerMethod<util::Vec2d (T::*)() const>(&T::getVelocity, "getVelocity");
	s.registerMethod<util::Vec2d (T::*)(util::Vec2d) const>(&T::getVelocity, "getVelocity");
	s.registerMethod<T>(&T::getEstimatedVelocity, "getEstimatedVelocity");

	s.registerMethod<T>(&T::setVelocity, "setVelocity");
	
	s.registerMethod<T>(&T::getAngularVelocity, "getAngularVelocity");
	s.registerMethod<T>(&T::setAngularVelocity, "setAngularVelocity");
	
	s.registerMethod<T>(&T::isActive, "isActive");
	s.registerMethod<T>(&T::setActive, "setActive");
	
	s.registerMethod<T>(&T::getMass, "getMass");
	s.registerMethod<T>(&T::getInvMass, "getInvMass");
	s.registerMethod<T>(&T::getInertia, "getInertia");
	
	s.registerMethod<T>(&T::addIgnoreObject, "addIgnoreObject");
	s.registerMethod<T>(&T::removeIgnoreObject, "removeIgnoreObject");

	s.registerMethod<T>(&T::getJoints, "getJoints");
	s.registerMethod<T>(&T::getGraphObjects, "getGraphObjects");

	s.registerMethod<T>(&T::getEntity, "getEntity");

	s.registerMethod<T>(&T::set3dTransform, "set3dTransform");
	s.registerMethod<T>(&T::get3dTransform, "get3dTransform");
	s.registerMethod<T>(&T::getEstimated3dTransform, "getEstimated3dTransform");

	s.registerMethod<T>(&T::setGhostliness, "setGhostliness");
	s.registerMethod<T>(&T::getGhostliness, "getGhostliness");

	s.registerMethod<T>(&T::isStatic, "isStatic");
}

template <typename T>
void registerJointMethods(){
	auto& s= *script::gScriptMgr;
	
	if (!std::is_same<Joint, T>::value)
		s.registerInheriting<Joint, T>();

	s.registerInheriting<OnAttachCb, T>();
	s.registerInheriting<OnDetachCb, T>();

	s.registerMethod<T>(&T::isAttached, "isAttached");
	s.registerMethod<T>(&T::detach, "detach");
	s.registerMethod<T>(&T::getObject, "getObject");
	s.registerMethod<T>(&T::getGraphObjects, "getGraphObjects");
	s.registerMethod<T>(&T::getGraphJoints, "getGraphJoints");
}

template <typename T>
void registerFixtureMethods(){
	auto& s= *script::gScriptMgr;
	
	s.registerMethod<T>(&T::getShape, "getShape");
	s.registerMethod<T>(&T::getObject, "getObject");
	s.registerMethod<T>(&T::getCallbacks, "getCallbacks");
}

void registerToScript(){
	auto& s= *script::gScriptMgr;

	// Shapes
	s.registerObjectType<collision::BaseShape*>();

	s.registerObjectType<collision::CircleBaseShape*>();
	s.registerInheriting<collision::BaseShape, collision::CircleBaseShape>();
	s.registerMethod(&collision::CircleBaseShape::setPosition, "setPosition");
	s.registerMethod(&collision::CircleBaseShape::setRadius, "setRadius");

	s.registerObjectType<collision::PolygonBaseShape*>();
	s.registerInheriting<collision::BaseShape, collision::PolygonBaseShape>();
	s.registerMethod(&collision::PolygonBaseShape::setVertices, "setVertices");
	s.registerMethod(&collision::PolygonBaseShape::setAsRect, "setAsRect");

	s.registerObjectType<collision::Shape*>();
	s.registerMethod(&collision::Shape::add, "add");
	s.registerMethod(&collision::Shape::empty, "empty");
	s.registerMethod(&collision::Shape::distance, "distance");
	resources::ResourceRef<collision::Shape>::registerToScript();

	// Physics
	s.registerObjectType<OnAttachCb*>();
	s.registerObjectType<OnDetachCb*>();

	util::CbListener<OnAttachCb>::registerToScript(s);
	util::CbListener<OnDetachCb>::registerToScript(s);

	// Because of interdependencies, these types must be registered before methods
	s.registerObjectType<Object>();
	s.registerObjectType<Joint>();
	s.registerObjectType<Entity>();

	registerJointMethods<Joint>();
	registerObjectMethods<Object>();

	// RigidObject
	s.registerObjectType<RigidObject>();
	s.registerInheriting<Object, RigidObject>();
	registerObjectMethods<RigidObject>();
	s.registerMethod(&RigidObject::setFixedRotation, "setFixedRotation");
	s.registerMethod(&RigidObject::hasFixedRotation, "hasFixedRotation");
	s.registerMethod(&RigidObject::setGravityScale, "setGravityScale");

	// Contact stuff
	s.registerObjectType<ContactSide>();
	s.registerMember(&ContactSide::object, "object");
	s.registerMember(&ContactSide::object, "fixture");
	s.registerObjectType<Contact>();
	s.registerMethod<ContactSide& (Contact::*)(uint32 i)>(&Contact::getSide, "getSide");
	s.registerMethod<const ContactSide& (Contact::*)(uint32 i) const>(&Contact::getSide, "getSide");
	s.registerObjectType<PostSolveContactSide>();
	s.registerMember(&PostSolveContactSide::totalImpulse, "totalImpulse");
	s.registerObjectType<PostSolveContact>();
	s.registerMethod<PostSolveContactSide& (PostSolveContact::*)(uint32 i)>(&PostSolveContact::getSide, "getSide");
	s.registerMethod<const PostSolveContactSide& (PostSolveContact::*)(uint32 i) const>(&PostSolveContact::getSide, "getSide");
	s.registerFuncdef<void (const Contact&)>();
	s.registerFuncdef<bool (const Contact&)>();
	s.registerFuncdef<void (const PostSolveContact&)>();
	s.registerObjectType<FixtureCallbacks>();
	s.registerMember(&FixtureCallbacks::onBeginContact, "onBeginContact");
	s.registerMember(&FixtureCallbacks::onEndContact, "onEndContact");
	s.registerMember(&FixtureCallbacks::onPreSolveContact, "onPreSolveContact");
	s.registerMember(&FixtureCallbacks::onPostSolveContact, "onPostSolveContact");
	
	// Material
	s.registerObjectType<Material>();
	
	// Traceable
	s.registerObjectType<collision::Traceable>();
	
	// Fixture
	s.registerObjectType<Fixture>();
	s.registerInheriting<collision::Traceable, Fixture>();
	registerFixtureMethods<Fixture>();
	
	// RigidFixtureDef
	s.registerObjectType<RigidFixtureDef>();
	s.registerMethod<void (RigidFixtureDef::*)(const util::Str8&)>(&RigidFixtureDef::setShape, "setShape");
	s.registerMethod<void (RigidFixtureDef::*)(const collision::Shape&)>(&RigidFixtureDef::setShape, "setShape");
	s.registerMethod<void (RigidFixtureDef::*)(const util::Str8&)>(&RigidFixtureDef::setMaterial, "setMaterial");
	s.registerMethod(&RigidFixtureDef::setDensity, "setDensity");
	s.registerMethod(&RigidFixtureDef::setRestitution, "setRestitution");
	s.registerMethod(&RigidFixtureDef::setFriction, "setFriction");
	s.registerMethod(&RigidFixtureDef::setAsSensor, "setAsSensor");
	
	// RigidFixture
	s.registerObjectType<RigidFixture>();
	s.registerInheriting<collision::Traceable, RigidFixture>();
	s.registerInheriting<Fixture, RigidFixture>();
	registerFixtureMethods<RigidFixture>();
	
	// RigidObject (continues)
	s.registerMethod(&RigidObject::add, "add");
	
			
	// HardWeldJoint
	s.registerObjectType<HardWeldJoint>();
	registerJointMethods<HardWeldJoint>();
	s.registerMethod<void (HardWeldJoint::*)(Object&, Object&)>(
			&HardWeldJoint::attach, "attach");
	s.registerMethod<void (HardWeldJoint::*)(Object&, Object&, WorldVec, WorldVec)>(
			&HardWeldJoint::attach, "attach");

	// RevoluteJoint
	s.registerObjectType<RevoluteJoint>();
	registerJointMethods<RevoluteJoint>();
	s.registerMethod<void (RevoluteJoint::*)(Object&, Object&, const WorldVec&)>(&RevoluteJoint::attach, "attach");
	s.registerMethod<void (RevoluteJoint::*)(Object&, const WorldVec&)>(&RevoluteJoint::attach, "attach");
	s.registerMethod(&RevoluteJoint::enableMotor, "enableMotor");
	s.registerMethod(&RevoluteJoint::isMotorEnabled, "isMotorEnabled");
	s.registerMethod(&RevoluteJoint::setMaxMotorTorque, "setMaxMotorTorque");
	s.registerMethod(&RevoluteJoint::setTargetAngularVelocity, "setTargetAngularVelocity");
	s.registerMethod(&RevoluteJoint::getTargetAngularVelocity, "getTargetAngularVelocity");
	
	// RopeJoint
	s.registerObjectType<RopeJoint>();
	registerJointMethods<RopeJoint>();
	s.registerMethod(&RopeJoint::attach, "attach");
	s.registerMethod(&RopeJoint::setLength, "setLength");

	// WeldJoint
	s.registerObjectType<WeldJoint>();
	registerJointMethods<WeldJoint>();
	s.registerMethod<void (WeldJoint::*)(Object&, Object&)>(&WeldJoint::attach, "attach");
	s.registerMethod(&WeldJoint::setFrequency, "setFrequency");
	s.registerMethod(&WeldJoint::setDamping, "setDamping");

	// WheelJoint
	s.registerObjectType<WheelJoint>();
	registerJointMethods<WheelJoint>();
	s.registerMethod<void (WheelJoint::*)(Object&, Object&, const WorldVec&)>(&WheelJoint::attach, "attach");
	s.registerMethod<void (WheelJoint::*)(Object&, const WorldVec&)>(&WheelJoint::attach, "attach");
	s.registerMethod(&WheelJoint::setSpringFrequency, "setSpringFrequency");
	s.registerMethod(&WheelJoint::setSpringDamping, "setSpringDamping");
	s.registerMethod(&WheelJoint::enableMotor, "enableMotor");
	s.registerMethod(&WheelJoint::isMotorEnabled, "isMotorEnabled");
	s.registerMethod(&WheelJoint::setMaxMotorTorque, "setMaxMotorTorque");
	s.registerMethod(&WheelJoint::setTargetAngularVelocity, "setTargetAngularVelocity");
	s.registerMethod(&WheelJoint::getTargetAngularVelocity, "getTargetAngularVelocity");

	// Entity
	s.registerMethod(&Entity::clear, "clear");
	s.registerMethod<void (Entity::*)(const util::SrtTransform3d&, const util::Str8&)>(&Entity::set, "set");
	s.registerMethod(&Entity::setAsRagdoll, "setAsRagdoll");
	s.registerMethod(&Entity::getRootObject, "getRootObject");
	s.registerMethod(&Entity::getPose, "getPose");
	s.registerMethod(&Entity::getEstimatedPose, "getEstimatedPose");
	s.registerMethod(&Entity::getEstimatedPosition, "getEstimatedPosition");
	s.registerMethod(&Entity::getTransform, "getTransform");
	s.registerMethod(&Entity::getEstimatedTransform, "getEstimatedTransform");
	s.registerMethod(&Entity::getJointTransform, "getJointTransform");
	s.registerMethod(&Entity::getEstimatedJointTransform, "getEstimatedJointTransform");

	
	// Queries
	s.registerGlobalFunction(pointQuery, "collision::traceablePointQuery");
	s.registerGlobalFunction(potentialRectQuery, "collision::traceablePotentialRectQuery");
	s.registerGlobalFunction(fixturePointQuery, "collision::fixturePointQuery");
	s.registerGlobalFunction(fixturePotentialRectQuery, "collision::fixturePotentialRectQuery");
	

	// ArmatureSuit
	s.registerObjectType<ArmatureSuit>();
	s.registerMethod(&ArmatureSuit::update, "update");
	s.registerMethod(&ArmatureSuit::get, "get");
	s.registerMethod(&ArmatureSuit::clear, "clear");
}

} // physics
} // clover
