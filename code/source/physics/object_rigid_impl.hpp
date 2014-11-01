#ifndef CLOVER_PHYSICS_OBJECT_RIGID_IMPL_HPP
#define CLOVER_PHYSICS_OBJECT_RIGID_IMPL_HPP

#include "build.hpp"
#include "physics/object.hpp"
#include "util/dyn_array.hpp"
#include "util/pooled_crtp.hpp"
#include "util/transform.hpp"
#include "util/unique_ptr.hpp"
#include "util/vector.hpp"

/// @todo Replace with util::Any
#include <boost/any.hpp>
#include <Box2D/Box2D.h>

namespace clover {
namespace collision {

class Shape;

} // collision
namespace physics {

class RigidObject;

enum class RigidObjectType {
		Dynamic,
		Static,
		Kinematic
};

/// RigidObject's definition
class RigidObjectDef {
public:

	RigidObjectDef(util::Vec2d pos=util::Vec2d{0,0}, real32 rot=0, util::Vec2d vel=util::Vec2d{0,0}, real32 angularvel=0);

	void setTransform(Object::Transform t);
	void setPosition(util::Vec2d pos);
	void setRotation(real64 rot);
	void setType(RigidObjectType t);

	/// Deprecated
	void setStatic(bool s= true);
	bool isStatic() const;

	void setPartiallyBreakable(bool b= true){ partiallyBreakable= b; }
	bool isPartiallyBreakable() const { return partiallyBreakable; }

	void setFixedRotation(bool s= true);
	void setCCD(bool s= true){ // Continuous collision detection
		def.bullet= s;
	}

	void setActive(bool a= true){ def.active= a; }
private:

	friend class RigidObject;

	b2BodyDef def;
	bool partiallyBreakable= false;
};



///User-defined data in Box2D body
struct b2BodyData {
	Object* owner;
	boost::any data;
};

class RigidFixtureDef;
class RigidFixture;

struct OnBreakCb : public util::SingleCallbacker<>{};

/// Should be only dynamically allocated because of pooling!
class RigidObject	: public Object
					, public util::Callbacker<OnBreakCb>
					, public util::PooledCrtp<RigidObject> {
public:

	typedef Object::Transform Transform;

	RigidObject(RigidObjectDef def= RigidObjectDef());
	DELETE_COPY(RigidObject);
	DELETE_MOVE(RigidObject);
	virtual ~RigidObject();

	/// Copies fix
	RigidFixture& add(const RigidFixtureDef& fix);
	void set(const RigidObjectDef& def);


	void setActive(bool a=true);
	bool isActive() const { return bodyDef.def.active; }

	void setStatic(bool s=true);
	bool isStatic() const { return bodyDef.def.type == b2_staticBody; }

	RigidObjectDef getDef(){ return bodyDef; }

	/// Destroys body and removes fixtures
	void clear();

	/// Removes and destroys fixtures, but not body
	void clearFixtures();

	virtual util::Vec2d getCenterOfMass() const override;
	/// @note Doesn't return correct value for non-dynamic objects!!!
	virtual real64 getMass() const override;
	virtual real64 getInertia() const override;
	virtual real64 getEffInvMass() const override;

	/// worldpoint is in world-space
	virtual void applyForce(util::Vec2d force, util::Vec2d worldpoint) override;
	virtual void applyForce(util::Vec2d force) override;
	virtual void applyImpulse(util::Vec2d impulse, util::Vec2d worldpoint) override;
	virtual void applyImpulse(util::Vec2d impulse) override;
	virtual void applyTorque(real64 torque) override;
	virtual void applyAngularImpulse(real64 impulse) override;

	virtual void setVelocity(const util::Vec2d& vel) override;
	using Object::getVelocity;
	virtual util::Vec2d getVelocity(util::Vec2d worldpoint) const override;

	virtual void setAngularVelocity(real64 omega) override;

	void setTransform(const Transform& t);
	void setPosition(util::Vec2d p);
	void setRotation(real32 r);

	void setFixedRotation(bool b= true);
	bool hasFixedRotation() const;
	void setGravityScale(real64 scale);

	bool isAwake() const;

	void setPartiallyBreakable(bool b= true) override { bodyDef.setPartiallyBreakable(b); }
	bool isPartiallyBreakable() const override { return  bodyDef.isPartiallyBreakable(); }

	real64 getBreakStress() const;
	void breakSome(const collision::Shape& break_area, bool boolean_and= false);
	void setFullyBroken();
	bool isFullyBroken() const { return fullyBroken; }

	template <typename T>
	void setCustomData(const T& data){ bodyData.data= data; }

	template <typename T>
	const T& getCustomData() const { return boost::any_cast<const T&>(bodyData.data); }

	/// @todo Convert return value to b2Body&
	b2Body* getB2Body() const;

	const util::DynArray<util::UniquePtr<RigidFixture>>& getRigidFixtures() const { return fixtures; }
	virtual util::DynArray<Fixture*> getFixtures() const override;

	/// Welds objects together like they were just one object
	/// Objects will become parts of the same uniform weld group
	/// @note These are helper functions for HardWeldJoint
	void weld(RigidObject& o);

	/// Breaks whole weld group
	void unweldGroup();

	virtual void setMirrored(bool b= true) override;
	virtual void setGhostliness(real64 g) override;

private:
	void preStepUpdate();
	void postStepUpdate();
public:

	static void preStepUpdateForAll();
	static void postStepUpdateForAll();
	/// @param relative_time See Object::updateEstimation
	static void updateEstimationsForAll(real64 relative_time);

protected:

	bool isProxy() const { return proxyData.get() != nullptr; }
	bool isProxyMaster() const { return proxyMasterData.get() != nullptr; }
	bool isNormal() const { return !isProxy() && !isProxyMaster(); }

private:
	void createB2Body();
	void destroyB2Body();

	util::Vec2d getB2Position() const;
	real64 getB2Rotation() const;
	util::Vec2d getB2Velocity() const;
	real64 getB2AngularVelocity() const;
	CachedValues getB2CachedValues() const;

	void overrideCachedValuesByB2();

	void recreateBody();
	void recreateFixtures();
	void createFixture(const RigidFixtureDef& fix_def);

	void onStaticnessChange();
	util::DynArray<RigidObject*> getWeldGroupObjects() const;

	void removeFromGrid();
	void addToGrid();

	/// Additional data for a proxy master - manager of a weld group
	struct ProxyMasterData {
		util::DynArray<RigidObject*> proxies;
		/// Center of mass offset
		util::Vec2d positionToCm;
		real64 mass;
		real64 inertia;
	};

	/// Additional data for a proxy
	struct ProxyData {
		RigidObject* master= nullptr;
		/// Offset to body of master
		Transform offset;
		util::Vec2d positionToCm;
		real64 mass;
		real64 inertia;
	};

	static util::LinkedList<RigidObject*> nonStaticRigidObjects;
	util::LinkedList<RigidObject*>::Iter nsSelfIter;

	RigidObjectDef bodyDef;

	// Pointers to avoid circular dependency
	util::DynArray<util::UniquePtr<RigidFixtureDef>> fixtureDefs;
	util::DynArray<util::UniquePtr<RigidFixture>> fixtures;
	bool fullyBroken= false;

	b2Body* body;
	b2BodyData bodyData;

	util::UniquePtr<ProxyMasterData> proxyMasterData;
	util::UniquePtr<ProxyData> proxyData;
};

} // physics
namespace util {

template <>
struct TypeStringTraits<physics::RigidFixtureDef> {
	static util::Str8 type(){ return "physics::RigidFixtureDef"; }
};

template <>
struct TypeStringTraits<physics::RigidObject> {
	static util::Str8 type(){ return "physics::RigidObject"; }
};

} // util
} // clover

#endif // CLOVER_PHYSICS_OBJECT_RIGID_IMPL_HPP
