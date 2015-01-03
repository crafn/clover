#include "animation/armature.hpp"
#include "collision/baseshape_polygon.hpp"
#include "entity.hpp"
#include "entity_def.hpp"
#include "fixture_rigid.hpp"
#include "global/env.hpp"
#include "joints.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace physics {
void Entity::set(const util::SrtTransform3d& transform, const EntityDef& def){
	clear();

	armature= &def.getArmature();
	animation::ArmaturePose::Pose pose= armature->getBindPose().getGlobalPose();

	// Create objects
	util::Map<util::Str8, Object*> nameToObject;
	for (const EntityObjectDef& obj_def : def.getObjectDefs()){
		util::SrtTransform3d t;

		animation::JointId joint_id= armature->findJointId(obj_def.name);
		if (joint_id != animation::JointIdNone){
			jointIdToObjectMap[joint_id]= objects.size();
			t= commonReplaced(t, pose[joint_id].transform)*transform;
		}

		RigidObjectDef rigid_def;
		rigid_def.setPosition(transform.translation.xy());
		RigidObject& obj= emplaceObject<RigidObject>(rigid_def);
		nameToObject[obj_def.name]= &obj;
		obj.set3dTransform(t);

		for (auto& fix : obj_def.fixtures){
			RigidFixtureDef fix_def;
			fix_def.setShape(fix.shape);
			fix_def.setMaterial(fix.material);
			obj.add(fix_def);
		}
	}

	// Create joints
	for (const EntityJointDef& joint_def : def.getJointDefs()){
		util::Vec2d anchor1, anchor2;
		if (joint_def.anchors.size() >= 1)
			anchor1= joint_def.anchors[0];
		if (joint_def.anchors.size() >= 2)
			anchor2= joint_def.anchors[1];

		if (joint_def.objects.empty())
			continue;

		for (auto& name : joint_def.objects){
			if (!util::contains(nameToObject, name))
				throw global::Exception("Joint has invalid object: %s", name.cStr());
		}

		Object* obj1= nullptr,
			  * obj2= nullptr;

		if (joint_def.objects.size() >= 1)
			obj1= nameToObject[joint_def.objects[0]];
		if (joint_def.objects.size() >= 2)
			obj2= nameToObject[joint_def.objects[1]];

		ensure(obj1 != nullptr);
		if (obj2 == nullptr)
			obj2= obj1;

		anchor1= anchor1*obj1->getTransform();
		anchor2= anchor2*obj2->getTransform();

		switch (joint_def.type){
			case JointType::Distance: {
				auto& j= emplaceJoint<DistanceJoint>();
				j.setFrequency(joint_def.frequency);
				j.setDamping(joint_def.damping);
				j.attach(*obj1, *obj2, anchor1, anchor2);
			} break;
			case JointType::Friction: {
				release_ensure_msg(0, "todo friction");
			} break;
			case JointType::HardWeld: {
				release_ensure_msg(0, "todo hardweld");
			} break;
			case JointType::Prismatic: {
				release_ensure_msg(0, "todo prismatic");
			} break;
			case JointType::Revolute: {
				release_ensure_msg(0, "todo revolute");
			} break;
			case JointType::Rope: {
				auto& j= emplaceJoint<RopeJoint>();
				j.attach(*obj1, *obj2, anchor1, anchor2);
			} break;
			case JointType::Weld: {
				release_ensure_msg(0, "todo weld");
			} break;
			case JointType::Wheel: {
				release_ensure_msg(0, "todo wheel");
			} break;
			case JointType::None: // Fall through
			case JointType::Last:  release_ensure_msg(0, "Invalid JointType"); 
		}

		Joint& joint= *joints.back();
		joint.setFrictionMul(joint_def.friction);
	}
}

void Entity::set(const util::SrtTransform3d& t, const util::Str8& entity_def_name){
	set(t, global::g_env->resCache->getResource<EntityDef>(entity_def_name));
}

void Entity::setActive(bool b){
	for (auto& obj : objects){
		obj->setActive(b);
	}
}

Object& Entity::getRootObject(const util::Str8& joint) const {
	ensure(armature);
	
	animation::JointId joint_id= armature->findJointId(joint);
	ensure(joint_id != animation::JointIdNone);

	// Traverse armature until joint with object assigned to it is found
	Object* obj= nullptr;
	while (joint_id != animation::JointIdNone){
		auto it= jointIdToObjectMap.find(joint_id);
		if (it != jointIdToObjectMap.end()){
			obj= objects.at(it->second).get();
			break;
		}

		joint_id= armature->getJoints().at(joint_id).getSuperId();
	}

	if (!obj)
		throw global::Exception("Object for joint not found: %s", joint.cStr());

	return *obj;
}

void Entity::setAsRagdoll(
		const util::RtTransform2d& transform,
		real64 thickness,
		const Material& mat,
		const animation::ArmaturePose& armature_pose){
	clear();
	ensure(objects.empty());
	
	armature= &armature_pose.getArmature();
	
	struct ObjectInfo {
		util::Vec2d start, end;
		real64 rotation, length;
		Object* object;
		animation::JointId superJointId;
	};
	
	// Objects by end joint
	util::Map<animation::JointId, ObjectInfo> object_infos;
	
	// Create bodies
	auto pose= armature_pose.getGlobalPose();
	for (animation::JointId i= 0; i < pose.size(); i++){
		animation::JointId super_i= armature_pose.getArmature().
			getJoints()[pose[i].jointId].getSuperId();
		
		util::Vec2d start, end;
		real64 rotation= 0.0, length= 0.0;
		RigidObject* object= nullptr;
		
		if (super_i == animation::JointIdNone){
			ensure(i < pose.size());
			// No super joint
			animation::JointPose::Transform& t_3d= pose[i].transform;
			util::RtTransform2d t(
					(real64)t_3d.rotation.rotationZ(),
					t_3d.translation.casted<util::Vec2d>().xy());
			
			start= end= (t*transform).translation;
			
			RigidObjectDef obj_def;
			obj_def.setPosition(start);
			object= &emplaceObject<RigidObject>(obj_def);
		}
		else {
			ensure(i < pose.size());
			ensure(super_i < pose.size());
			animation::JointPose::Transform& end_t_3d= pose[i].transform;
			animation::JointPose::Transform& start_t_3d= pose[super_i].transform;
			
			util::RtTransform2d end_t(
					(real64)end_t_3d.rotation.rotationZ(),
					end_t_3d.translation.casted<util::Vec2d>().xy());
			util::RtTransform2d start_t(
					(real64)start_t_3d.rotation.rotationZ(),
					start_t_3d.translation.casted<util::Vec2d>().xy());
			
			start= (start_t*transform).translation;
			end= (end_t*transform).translation;
			rotation= (end - start).rotationZ();
			
			util::DynArray<util::Vec2d> vertices;
			vertices.resize(4);

			length= (end.xy() - start.xy()).length();

			vertices[0]= util::Vec2d{-length*0.5, thickness};
			vertices[1]= util::Vec2d{-length*0.5, -thickness};

			vertices[2]= util::Vec2d{length*0.5, -thickness};
			vertices[3]= util::Vec2d{length*0.5, thickness};

			collision::PolygonBaseShape quad;
			quad.setVertices(vertices);
			
			collision::Shape& shape= emplaceShape();
			shape.add(quad);
			
			RigidFixtureDef fix_def;
			fix_def.setShape(shape);
			fix_def.setMaterial(mat);

			RigidObjectDef obj_def;
			obj_def.setPosition((start + end)*0.5);
			obj_def.setRotation(rotation);
			object= &emplaceObject<RigidObject>(obj_def);
			object->add(fix_def);

			for (const auto& pair : object_infos)
				object->addIgnoreObject(*pair.second.object); // Disable self collision
		}

		ensure(object);
		object_infos[i]= ObjectInfo({start, end, rotation, length, object, super_i});
		jointIdToObjectMap[i]= i;
	}

	// Create joints
	for (auto& object_info_pair : object_infos){
		auto& object_info= object_info_pair.second;
		Object* first_object= object_info.object;
		
		if (object_info.superJointId == animation::JointIdNone)
			continue;
		
		Object* second_object= object_infos[object_info.superJointId].object;
		
		ensure(first_object); ensure(second_object);
		util::Vec2d anchor_point= object_info.start;
		
		auto& joint= emplaceJoint<RevoluteJoint>();
		joint.attach(*first_object, *second_object, anchor_point);
	}
}

collision::Shape& Entity::emplaceShape(){
	shapes.pushBack(collision::Shape());
	return shapes.back();
}

animation::ArmaturePose Entity::getPose() const {
	return getExactPose(&Object::get3dTransform);
}

animation::ArmaturePose Entity::getEstimatedPose() const {
	return getExactPose(&Object::getEstimated3dTransform);
}

util::Vec2d Entity::getCenterOfMass() const {
	real64 total_mass= 0.0;
	for (const auto& ob : objects)
		total_mass += ob->getMass();
	
	ensure(total_mass > 0.0);
	
	util::Vec2d pos;
	for (const auto& ob : objects)
		pos += ob->getCenterOfMass()*ob->getMass()/total_mass;
	
	return pos;
}

util::Vec2d Entity::getEstimatedPosition() const {
	util::Vec2d pos;
	for (const auto& ob : objects)
		pos += ob->getEstimatedTransform().translation*1.0/objects.size();
	return pos;
}

util::SrtTransform3d Entity::getTransform() const {
	return getExactTransform(&Object::get3dTransform);
}

util::SrtTransform3d Entity::getEstimatedTransform() const {
	return getExactTransform(&Object::getEstimated3dTransform);
}

util::SrtTransform3d Entity::getJointTransform(const util::Str8& joint) const {
	return getExactJointTransform(joint, &Object::get3dTransform);
}

util::SrtTransform3d Entity::getEstimatedJointTransform(const util::Str8& joint) const {
	return getExactJointTransform(joint, &Object::getEstimated3dTransform);
}

void Entity::clear(){
	objects.clear();
	joints.clear();
	shapes.clear();
	
	armature= nullptr;
	jointIdToObjectMap.clear();
}

template <typename T>
T Entity::getExactTransform(
			T (Object::* obj_transform)() const) const {
	if (!armature || jointIdToObjectMap.empty())
		return T{}; /// @todo
	
	// Pick first joint we have an object for, and use it to figure out
	// transform relative to origo of the armature in bind pose.
	// Usually root joint is the first
	auto& joints= armature->getJoints();
	for (auto& joint : joints){
		auto it= jointIdToObjectMap.find(joint.getId());
		if (it == jointIdToObjectMap.end())
			continue;

		const Object& o= getObject(it->second);
		return commonReplaced(T{}, joint.getBindPose().transform)*
			(o.*obj_transform)();
	}

	release_ensure_msg(0, "this should not happen");
	return T{};
}

template util::RtTransform2d Entity::getExactTransform(
		util::RtTransform2d (Object::*)() const) const;
template util::SrtTransform3d Entity::getExactTransform(
		util::SrtTransform3d (Object::*)() const) const;

util::SrtTransform3d Entity::getExactJointTransform(
			const util::Str8& joint,
			util::SrtTransform3d (Object::* obj_transform)() const) const {
	ensure(armature);

	animation::JointId joint_id= armature->getJointId(joint);
	ensure(joint_id != animation::JointIdNone);

	auto joint_t= commonReplaced(
			util::SrtTransform3d{},
			getExactPose(obj_transform).
				getGlobalPose().at(joint_id).transform);
	return joint_t*getExactTransform(obj_transform);

}


animation::ArmaturePose Entity::getExactPose(
			util::SrtTransform3d (Object::* obj_transform)() const) const {
	ensure(armature);
	
	animation::ArmaturePose::Pose pose= armature->getBindPose().getGlobalPose();
	util::SrtTransform3d origo= getExactTransform(obj_transform);
	
	/// @todo Transform subjoints which doesn't have attached objects
	for (const auto& pair : jointIdToObjectMap){
		auto joint_id= pair.first;
		auto object_id= pair.second;
		
		ensure(joint_id < pose.size());
		ensure(object_id < objects.size());
		
		util::SrtTransform3d joint_t=
			(objects.at(object_id).get()->*obj_transform)()*origo.inversed();
		pose[joint_id].transform.translation= joint_t.translation.casted<util::Vec3f>();
		pose[joint_id].transform.rotation= joint_t.rotation.casted<util::Quatf>();
		pose[joint_id].transform.scale= joint_t.scale.x;
	}
	
	return animation::ArmaturePose::byGlobalPose(*armature, pose);

}

} // physics
} // clover
