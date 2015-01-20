#ifndef CLOVER_PHYSICS_FIXTURE_RIGID_HPP
#define CLOVER_PHYSICS_FIXTURE_RIGID_HPP

#include "build.hpp"
#include "collision/baseshape.hpp"
#include "collision/shape.hpp"
#include "physics/b2_fixtureuserdata.hpp"
#include "physics/fixture.hpp"
#include "resources/resource_ref.hpp"
#include "util/transform.hpp"
#include "util/dyn_array.hpp"
#include "physics/object_rigid_impl.hpp"

namespace clover {
namespace physics {

/// Shape + properties which can be added to RigidObject
class ENGINE_API RigidFixtureDef final : public FixtureDef {
public:

	RigidFixtureDef(real32 density=1, real32 friction=0.5, real32 restitution=0.5);
	RigidFixtureDef(const physics::Material& mat);
	RigidFixtureDef(const RigidFixtureDef& f);
	RigidFixtureDef(RigidFixtureDef&&);
	virtual ~RigidFixtureDef();

	virtual void onEvent(global::Event& e);

	/// Sets the shape of the fixture
	void setShape(const collision::Shape& shape);
	void setShape(resources::ResourceRef<collision::Shape> shape);
	void setShape(const util::Str8& name);
	const collision::Shape* getShape() const { return shape.isSet() ? &shape.get() : nullptr; }

	/// Doesn't alter the physics::Material, doesn't launch any change-events
	/// Don't use these unless really necessary; these aren't data-driven-friendly
	void setDensity(real32 f);
	void setRestitution(real32 f);
	void setFriction(real32 f);

	virtual void setMaterial(const physics::Material& mat);
	void setMaterial(const util::Str8& name);

	void setAsSensor(bool b= true);
	bool isSensor() const;

	const b2FixtureDef& getB2Def() const { return box2dDef; }


private:
	friend class RigidObject;

	b2FixtureDef box2dDef;
	resources::ResourceRef<collision::Shape> shape;
};

class RigidObject;

class ENGINE_API RigidFixture final : public Fixture {
public:
	using Transform= Fixture::Transform;
	using Def= RigidFixtureDef;

	static constexpr collision::Traceable::TypeId staticTraceableTypeId= collision::Traceable::RigidFixture;

	/// @param owner is the object for which the underlying fixture is created
	/// @param object is the object which is has the fixture as seen by user
	/// Usually they're the same, but in the case of proxies they aren't
	RigidFixture(RigidObject& owner, RigidObject& object, const RigidFixtureDef& def);
	RigidFixture(const RigidFixture&)= delete;
	RigidFixture(RigidFixture&&)= delete;
	virtual ~RigidFixture();

	virtual Transform getTransform() const;

	virtual bool overlaps(const util::Vec2d& point) const;
	virtual bool overlaps(const Traceable& other) const;
	virtual bool overlaps(const collision::Ray& ray, collision::RayCastResult* output=0) const;
	bool overlaps(const collision::BaseShape& shp, const Transform& shp_transform= Transform()) const;

	bool isSensor() const;

	void setShape(resources::ResourceRef<collision::Shape> shape);
	const collision::Shape* getShape() const override { return def.getShape(); }

	virtual RigidObject& getObject() const override { ensure(object); return *object; }

	void changeOwner(RigidObject& new_owner);
	void recreate(){ recreateB2Fixtures(); }

private:
	void recreateB2Fixtures();
	void destroyB2Fixtures();
	void refreshListeners();

	RigidObject* owner;
	RigidObject* object;
	RigidFixtureDef def;

	util::DynArray<b2Fixture*> box2dFixtures;

	B2FixtureUserData userData;

	util::CbListener<util::OnChangeCb> shapeChangeListener;
	util::CbListener<util::OnChangeCb> materialChangeListener;
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_FIXTURE_RIGID_HPP
