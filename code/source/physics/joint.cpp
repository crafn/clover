#include "joint.hpp"
#include "friction.hpp"
#include "object.hpp"
#include "util/graph.hpp"

namespace clover {
namespace physics {

static util::LinkedList<const Joint*> fJoints;

Joint::Joint()
		: type(JointType::None)
		, fJointsIt(fJoints.end()){
}

Joint::~Joint(){
	ensure_msg(objects.empty(), "Detach joints in derived destructors");

	if (fJointsIt != fJoints.end())
		fJoints.erase(fJointsIt);
}

void Joint::detach(){
	if (!objects.empty())
		OnDetachCb::trigger();
	
	for (int32 i=0; i<(int)objects.size(); i++){
		objects[i]->removeJoint(*this);
	}
	
	objects.clear();
}

util::DynArray<Object*> Joint::getGraphObjects() const {
	return util::findGraphNodes<util::DynArray<Object*>, Object*>(
			objects,
			[] (Object* node) -> util::DynArray<Object*> {
				util::DynArray<Object*> objs;
				for(auto j : node->getJoints())
					objs.pushBack(j->getObjects());
				return objs;
			}
	);
}

util::DynArray<Joint*> Joint::getGraphJoints(){
	return util::findGraphNodes<util::DynArray<Joint*>, Joint*>(
			util::DynArray<Joint*>{ this },
			[] (Joint* node) -> util::DynArray<Joint*> {
				return node->getNeighborJoints();
			}
		);
	return {};
}

void Joint::setFrictionMul(real64 mul){
	frictionMul= mul;
	onFrictionChange();
}

void Joint::onFrictionChange() const {
	bool has_friction= frictionMul != 0.0;

	if (has_friction && fJointsIt == fJoints.end()){
		fJointsIt= fJoints.insert(fJoints.end(), this);
	}
	else if (!has_friction && fJointsIt != fJoints.end()){
		fJoints.erase(fJointsIt);
		fJointsIt= fJoints.end();
	}
}

void Joint::simulateFriction(real64 dt) const {
	if (objects.size() < 2)
		return;

	// Not sure if forces should be applied to anchors...
	applyFriction(
			*NONULL(objects[0]),
			*NONULL(objects[1]),
			frictionMul,
			dt);
}

void Joint::simulateFrictions(real64 dt){
	for (const Joint* joint : fJoints){
		joint->simulateFriction(dt);
	}
}

util::DynArray<Joint*> Joint::getNeighborJoints() const {
	util::DynArray<Joint*> connected;
	for (SizeType i= 0; i < getObjectCount(); ++i){
		for (Joint* j : getObject(i).getJoints()){
			ensure(j);
			if (!util::contains(connected, j))
				connected.pushBack(j);
		}
	}
	return connected;
}


void Joint::addObject(Object& o){
	objects.pushBack(&o);
	o.addJoint(*this);
}

} // physics
} // clover