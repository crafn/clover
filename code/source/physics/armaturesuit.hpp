#ifndef CLOVER_PHYSICS_ARMATURESUIT_HPP
#define	CLOVER_PHYSICS_ARMATURESUIT_HPP

#include "animation/armaturepose.hpp"
#include "build.hpp"
#include "object_rigid.hpp"
#include "util/class_preproc.hpp"
#include "util/map.hpp"
#include "util/pair.hpp"
#include "util/staticlist_crtp.hpp"
#include "util/unique_ptr.hpp"

namespace clover {
namespace physics {

class PhysMgr;

/// A set of empty objects which follow the joints of an armature
class ArmatureSuit : public util::StaticListCrtp<ArmatureSuit> {
public:
	ArmatureSuit()= default;
	DELETE_COPY(ArmatureSuit);
	DEFAULT_MOVE(ArmatureSuit);

	void update(const util::SrtTransform3d& t, const animation::ArmaturePose& pose);
	RigidObject& get(const util::Str8& joint) const;
	void clear();

protected:
	friend class PhysMgr;
	static void preStepUpdates(real64 dt);
	void preStepUpdate(real64 dt);

private:
	void createObjects(
			const util::SrtTransform3d& t,
			const animation::ArmaturePose& pose);
	void updateEstimations();

	struct Sample {
		util::SrtTransform3d transform;
		animation::ArmaturePose::Pose globalPose;
	};

	Sample sample;

	const animation::Armature* armature= nullptr;
	util::Map<util::Str8, util::UniquePtr<RigidObject>> objects;
};

} // physics
namespace util {

template <>
struct TypeStringTraits<physics::ArmatureSuit> {
	static util::Str8 type(){ return "physics::ArmatureSuit"; }
};

} // util
} // clover

#endif // CLOVER_PHYSICS_ARMATURESUIT_HPP
