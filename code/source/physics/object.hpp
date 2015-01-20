#ifndef CLOVER_PHYSICS_OBJECT_HPP
#define CLOVER_PHYSICS_OBJECT_HPP

#include "build.hpp"
#include "util/callbacker.hpp"
#include "util/class_preproc.hpp"
#include "util/properties.hpp"
#include "global/exception.hpp"
#include "util/dyn_array.hpp"
#include "util/math.hpp"
#include "util/transform.hpp"

#include <algorithm>

namespace clover {
namespace physics {

class Entity;
class Fixture;
class Joint;
class PhysMgr;

/// Physics object
class ENGINE_API Object : public util::Callbacker<util::OnDestroyCb> {
public:
	using Transform= util::RtTransform2d;
	using Transform3= util::SrtTransform3d;

	enum class TypeId {
		None,
		Rigid,
		LastTypeId
	};

	Object()= default;
	DELETE_COPY(Object);
	DELETE_MOVE(Object);
	virtual ~Object();

	TypeId getTypeId() const { return typeId; }


	virtual void setActive(bool active=true)=0;
	virtual bool isActive() const = 0;

	virtual bool isStatic() const = 0;
	virtual bool hasFixedRotation() const = 0;

	/// Not necessarily mass center
	Transform getTransform() const { return cachedCurrent.transform; }
	util::Vec2d getPosition() const { return cachedCurrent.transform.translation; }
	real64 getRotation() const { return cachedCurrent.transform.rotation; }
	virtual util::Vec2d getCenterOfMass() const = 0;
	virtual real64 getMass() const = 0;
	real64 getInvMass() const;

	/// @return The effective mass of the object
	/// If object is static or hardwelded getEffInvMass != getMass
	virtual real64 getEffInvMass() const = 0;

	virtual void setVelocity(const util::Vec2d& vel)= 0;
	/// Average velocity
	util::Vec2d getVelocity() const { return cachedCurrent.velocity.translation; }
	virtual util::Vec2d getVelocity(util::Vec2d worldpoint) const = 0;

	util::Vec2d getMomentum() const { return getVelocity()*getMass(); }

	virtual real64 getInertia() const = 0;
	virtual void setAngularVelocity(real64 omega) = 0;
	virtual real64 getAngularVelocity() const { return cachedCurrent.velocity.rotation; }
	real64 getAngularMomentum() const { return getAngularVelocity()*getInertia(); }

	/// @return Total impulse applied to body in previous physics timestep
	util::RtTransform2d getImpulse() const;

	/// @return Inter- or extrapolated transform
	Transform getEstimatedTransform() const { return estimated.transform; }
	Transform getEstimatedVelocity() const { return estimated.velocity; }
	/// @note	Updates also estimated transform of hardwelded objects,
	///			because they're inseparable from each other
	void setEstimatedTransform(const Transform& t);

	virtual void applyForce(util::Vec2d force, util::Vec2d worldpoint)= 0;
	virtual void applyForce(util::Vec2d force)= 0;
	virtual void applyTorque(real64 torque)= 0;

	virtual void applyImpulse(util::Vec2d impulse)= 0;
	virtual void applyImpulse(util::Vec2d impulse, util::Vec2d worldpoint)= 0;
	virtual void applyAngularImpulse(real64 impulse)= 0;

	virtual void setTransform(const Transform& t)= 0;
	virtual void setPosition(util::Vec2d p)=0;
	virtual void setRotation(real32 r)=0;

	virtual void setPartiallyBreakable(bool b= true)= 0;
	virtual bool isPartiallyBreakable() const = 0;

	virtual util::DynArray<Fixture*> getFixtures() const = 0;

	void detachJoints();

	/// @return Entity in which this object belongs
	Entity* getEntity() const { return entity; }

	void addIgnoreObject(const Object& o);
	void removeIgnoreObject(const Object& o);

	/// Called internally
	void addJoint(Joint& c);
	void removeJoint(Joint& c);

	const util::DynArray<Joint*>& getJoints() const { return joints; }
	util::DynArray<Object*> getGraphObjects() const;

	/// Support for 3d transform so that attaching to armatures work correctly
	/// Third dimension doesn't affect simulation in any way
	void set3dTransform(const Transform3& t);
	void set3dOffset(const Transform3& t);
	Transform3 get3dTransform() const;
	Transform3 getEstimated3dTransform() const;

	/// @return Whether physics object is mirrored along x-axis
	/// Mirroring happens when object has 3d rotation which would reveal the
	/// other side of the object
	bool isMirrored() const { return mirrored; }
	/// Consider using set3dOffset or set3dTransform before flipping manually
	virtual void setMirrored(bool b= true){ mirrored= b; }

	/// When ghostliness ==
	///		- 0.0: normal behavior
	///		- 1.0: nothing collides
	///		- 0.5: collides, but sliding through is possible
	virtual void setGhostliness(real64 g){ ghostliness= g; }
	real64 getGhostliness() const { return ghostliness; }

	util::Properties& getProperties(){ return properties; }
	const util::Properties& getProperties() const { return properties; }

protected:
	friend class Entity;
	void setEntity(Entity* e){ entity= e; }

	struct CachedValues {
		Transform transform, velocity;
	};

	void resetCachedValues(const CachedValues& values);
	void updateCachedValues(const CachedValues& values);

	/// Updates inter- or extrapolated transform
	/// @param relative_time	Time relative to current physics simulation time scaled by 1/timeStep
	///							[-1, 0]: interpolation between previous and current state
	///							]0, 1[: extrapolation
	void updateEstimation(real64 relative_time);


	TypeId typeId= TypeId::None;
private:
	friend class PhysMgr;

	Entity* entity= nullptr;
	util::DynArray<Joint*> joints;
	CachedValues cachedCurrent, cachedPrevious, estimated;

	Transform3 offset3d;
	int32 offset3dRestoreCounter= 0;
	bool mirrored= false;

	real64 ghostliness= 0.0;

	/// User-controlled properties
	util::Properties properties;
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_OBJECT_HPP
