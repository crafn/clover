#ifndef CLOVER_PHYSICS_ENTITY_HPP
#define CLOVER_PHYSICS_ENTITY_HPP

#include "build.hpp"
#include "collision/shape.hpp"
#include "joint.hpp"
#include "material.hpp"
#include "object.hpp"
#include "util/properties.hpp"
#include "util/unique_ptr.hpp"

namespace clover {
namespace animation {

class Armature;
class ArmaturePose;

} // animation
namespace physics {

class EntityDef;

/// A collection of objects, joints and shapes
class Entity : public script::NoCountReference {
public:

	Entity()= default;
	Entity(const Entity&)= delete;
	Entity(Entity&&)= default;
	
	Entity& operator=(const Entity&)= delete;
	Entity& operator=(Entity&&)= default;

	void set(const util::SrtTransform3d& t, const EntityDef& def);
	void set(const util::SrtTransform3d& t, const util::Str8& entity_def_name);

	void setActive(bool b= true);

	template <typename T>
	T& emplaceObject();
	
	Object& getObject(SizeType i) const { return *objects.at(i); }

	/// @return Object which determines position of 'joint'
	Object& getRootObject(const util::Str8& joint) const;
	
	template <typename T>
	T& emplaceJoint();
	
	collision::Shape& emplaceShape();
	
	/// Creates ragdoll. Doesn't support multiple ragdolls or ragdolls & objects.
	void setAsRagdoll(
			const util::RtTransform2d& t,
			real64 thickness,
			const Material& mat,
			const animation::ArmaturePose& pose);
	
	/// Calculates and returns a pose in relative to transform
	animation::ArmaturePose getPose() const;
	animation::ArmaturePose getEstimatedPose() const;

	/// Center of mass in world coordinates
	util::Vec2d getCenterOfMass() const;
	
	/// Average of estimated positions
	util::Vec2d getEstimatedPosition() const;
	
	/// World coordinates
	util::SrtTransform3d getTransform() const;
	util::SrtTransform3d getEstimatedTransform() const;

	/// World coordinates
	util::SrtTransform3d getJointTransform(const util::Str8& joint) const;
	util::SrtTransform3d getEstimatedJointTransform(const util::Str8& joint) const;

	/// Removes all objects, joints and shapes
	void clear();

	bool hasArmature() const { return armature != nullptr; }

	util::Properties& getProperties(){ return properties; }
	const util::Properties& getProperties() const { return properties; }

private:
	/// set(getExactTransform()) doesn't change transform
	template <typename T>
	T getExactTransform(
			T (Object::* obj_transform)() const) const;

	util::SrtTransform3d getExactJointTransform(
			const util::Str8& joint,
			util::SrtTransform3d (Object::* obj_transform)() const) const;

	animation::ArmaturePose getExactPose(
			util::SrtTransform3d (Object::* obj_transform)() const) const;


	util::DynArray<util::UniquePtr<Object>> objects;
	util::DynArray<util::UniquePtr<Joint>> joints;
	util::LinkedList<collision::Shape> shapes;
	
	const animation::Armature* armature= nullptr;
	using ObjectId= SizeType;
	util::Map<animation::JointId, ObjectId> jointIdToObjectMap;

	/// User-controlled properties
	util::Properties properties;
};

template <typename T>
T& Entity::emplaceObject(){
	objects.pushBack(util::UniquePtr<Object>(new T()));
	objects.back()->setEntity(this);
	return *static_cast<T*>(objects.back().get());
}

template <typename T>
T& Entity::emplaceJoint(){
	joints.pushBack(util::UniquePtr<Joint>(new T()));
	return *static_cast<T*>(joints.back().get());
}

} // physics
namespace util {

template <>
struct TypeStringTraits<physics::Entity> {
	static util::Str8 type(){ return "physics::Entity"; }
};

} // util
} // clover

#endif // CLOVER_PHYSICS_ENTITY_HPP
