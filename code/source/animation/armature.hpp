#ifndef CLOVER_ANIMATION_ARMATURE_HPP
#define CLOVER_ANIMATION_ARMATURE_HPP

#include "armaturepose.hpp"
#include "build.hpp"
#include "joint.hpp"
#include "resources/resource.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace animation {

class Armature;

} // animation
namespace resources {

template <>
struct ResourceTraits<animation::Armature> {
	DECLARE_RESOURCE_TRAITS(animation::Armature, String);

	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"))

	typedef SubCache<animation::Armature> SubCacheType;

	static util::Str8 typeName(){ return "Armature"; }
	static util::Str8 stringFromIdentifier(const IdentifierValue& value){ return value; }
	static util::Str8 identifierKey(){ return "name"; }
	static bool expired(const animation::Armature&){ return false; }

};

} // resources
namespace animation {

/// Contains a hierarchial set of joints
class ENGINE_API Armature : public resources::Resource {
public:
	DECLARE_RESOURCE(Armature)

	using Joints= util::DynArray<Joint>;

	Armature();
	Armature(const Armature&)= delete;
	Armature(Armature&&)= delete;
	virtual ~Armature(){}

	Armature& operator=(const Armature&)= delete;
	Armature& operator=(Armature&&)= delete;

	const util::Str8& getName() const { return nameAttribute.get(); }

	/// JointIds are invalidated when resource state changes
	JointId getJointId(const util::Str8& name) const;
	JointId findJointId(const util::Str8& name) const;
	const Joints& getJoints() const { return joints; }

	const ArmaturePose& getBindPose() const { return bindPose; }

	virtual void resourceUpdate(bool load, bool force= true) override;
	virtual void createErrorResource() override;

private:
	RESOURCE_ATTRIBUTE(String, nameAttribute);
	RESOURCE_ATTRIBUTE(JointDefArray, jointDefsAttribute);
	RESOURCE_ATTRIBUTE(Path, importAttribute);

	Joints joints;
	ArmaturePose bindPose;
};

using JointNameToIdMap= util::HashMap<util::Str8, JointId>;
/// @return ["root"] == root_joint_id
JointNameToIdMap jointNameToIdMap(const Armature& a);

/// @return [from_jointId] == to_jointId
util::DynArray<JointId> jointIdMapForArmatures(const Armature& from, const Armature& to);

} // animation
namespace util {

template <>
struct TypeStringTraits<animation::Armature> {
	static util::Str8 type(){ return "animation::Armature"; }
};

} // util
} // clover

#endif // CLOVER_ANIMATION_ARMATURE_HPP