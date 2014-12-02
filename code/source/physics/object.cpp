#include "object.hpp"
#include "debug/debugdraw.hpp"
#include "fixture.hpp"
#include "global/event.hpp"
#include "joint.hpp"
#include "jointtype.hpp"
#include "util/graph.hpp"

namespace clover {
namespace physics {

Object::~Object(){
	util::OnDestroyCb::trigger();
	detachJoints();
}

real64 Object::getInvMass() const {
	real64 mass= getMass();
	return mass == 0.0 ? 0.0 : 1.0/mass;
}

util::RtTransform2d Object::getImpulse() const {
	auto vel_dif= cachedCurrent.velocity*cachedPrevious.velocity.inversed();
	return util::RtTransform2d(	vel_dif.rotation*getAngularMomentum(),
							vel_dif.translation*getMomentum());
}

void Object::setEstimatedTransform(const Transform& t){
	// Find objects which are hardwelded to this object
	util::DynArray<Object*> objects=
		util::findGraphNodes<util::DynArray<Object*>, Object*>(
			{this},
			[] (Object* node) -> util::DynArray<Object*> {
				util::DynArray<Object*> objs;
				for(auto j : node->getJoints()){
					if (j->getType() == JointType::HardWeld)
						objs.pushBack(j->getObjects());
				}
				return objs;
			}
	);

	for (Object* obj : objects){
		auto offset_t= obj->getTransform()*getTransform().inversed();
		obj->estimated.transform= offset_t*t;
	}
}


void Object::detachJoints(){
	while (!joints.empty())
		joints.front()->detach();
}

void Object::addIgnoreObject(const Object& o){
	if (&o == this)
		return;

	for (auto& fix : getFixtures()){
		for (const auto& other_fix : o.getFixtures()){
			fix->getContactFilter().addIgnoreFixture(*other_fix);
		}
	}
}

void Object::removeIgnoreObject(const Object& o){
	if (&o == this)
		return;

	for (auto& fix : getFixtures()){
		for (const auto& other_fix : o.getFixtures()){
			if (fix->getContactFilter().hasIgnoreFixture(*other_fix))
				fix->getContactFilter().removeIgnoreFixture(*other_fix);
		}
	}
}

void Object::addJoint(Joint& c){
	joints.pushBack(&c);

}

void Object::removeJoint(Joint& c){
	joints.erase(std::remove(joints.begin(), joints.end(), &c), joints.end());
}

util::DynArray<Object*> Object::getGraphObjects() const {
	if (joints.empty())
		return util::DynArray<Object*>{ const_cast<Object*>(this) };
	
	return joints.front()->getGraphObjects();
}

void Object::set3dTransform(const Transform3& t_3d){
	Transform t_2d= commonReplaced(Transform{}, t_3d);
	if (!joints.empty()){
		/*print(debug::Ch::General, debug::Vb::Trivial, "rot: (%f, %f, %f), %f",
				t_3d.rotation.axis().x, t_3d.rotation.axis().y, t_3d.rotation.axis().z,
				t_3d.rotation.rotation());
		util::Vec3d v= util::Vec3d{1, 0, 0}*t_3d.rotation;
		print(debug::Ch::General, debug::Vb::Trivial, "vec: (%f, %f, %f)",
			v.x, v.y, v.z);*/

		debug::gDebugDraw->addLine(t_2d.translation, t_2d.translation + 
				util::Vec2d{cos(t_2d.rotation), sin(t_2d.rotation)});
	}
	setTransform(t_2d);
	set3dOffset(t_3d*commonReplaced(Transform3{}, t_2d.inversed()));
}

void Object::set3dOffset(const Transform3& t){
	offset3d= t;
	util::Vec2d in_pos= getPosition();

	// This causes e.g. entity in hand to follow the empty object
	// in ArmatureSuit correctly
	// Consider instantaneous movement according to offset as
	// part of the physics in the game world..!
	/// @todo	Smoothing: if two objects in a net of objects are
	///			set3dTransformed sequentially, jittering occurs
	/// @todo	Don't set offsets over Entity boundaries
	for (Object* o : getGraphObjects()){
		if (o != this)
			o->offset3d= offset3d;
		
		o->offset3dRestoreCounter= 3;

		bool should_be_mirrored= (util::Vec3d{0, 0, 1}*t.rotation).z < 0;
		if (o->isMirrored() != should_be_mirrored && o == this){
			o->setMirrored(!o->isMirrored());
		}
	}

	util::Vec2d out_pos= getPosition();
	ensure( (in_pos - out_pos).lengthSqr() < 0.01);
}

Object::Transform3 Object::get3dTransform() const {
	return offset3d*commonReplaced(Transform3{}, getTransform());
}

Object::Transform3 Object::getEstimated3dTransform() const {
	return offset3d*commonReplaced(Transform3{}, getEstimatedTransform());
}

void Object::resetCachedValues(const CachedValues& values){
	cachedCurrent= cachedPrevious= estimated= values;
}

void Object::updateCachedValues(const CachedValues& values){
	cachedPrevious= cachedCurrent;
	cachedCurrent= values;

	// Not really right place for this or correct calculation, but
	// since this is only a visual thing, this is good enough for now
	if (offset3dRestoreCounter <= 0)
		offset3d.translation *= 0.9;
	else
		--offset3dRestoreCounter;
}

void Object::updateEstimation(real64 relative_time){
	if (relative_time < -1.0 || relative_time > 1.0){
		// Invalid value
		return;
	}
	
	// Linear inter- or extrapolation
	estimated.transform= lerp(cachedPrevious.transform, cachedCurrent.transform, 1.0 + relative_time);
	estimated.velocity= lerp(cachedPrevious.velocity, cachedCurrent.velocity, 1.0 + relative_time);
}

} // physics
} // clover
