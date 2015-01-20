#ifndef CLOVER_PHYSICS_FIXTURE_HPP
#define CLOVER_PHYSICS_FIXTURE_HPP

#include "build.hpp"
#include "collision/traceable.hpp"
#include "global/eventreceiver.hpp"
#include "global/module_util.hpp"
#include "physics/object.hpp"
#include "physics/contact.hpp"
#include "physics/material.hpp"
#include "util/callbacker.hpp"
#include "util/countedpointer.hpp"

namespace clover {
namespace collision {

class Shape;

} // collision
namespace physics {

struct FixtureCallbacks {
	using OnBeginContact= MOD_FPTR_TYPE(void (*)(const Contact&, void*));
	using OnEndContact= MOD_FPTR_TYPE(void (*)(const Contact&, void*));
	using OnPreSolveContact= MOD_FPTR_TYPE(bool (*)(const Contact&, void*));
	using OnPostSolveContact= MOD_FPTR_TYPE(void (*)(const PostSolveContact&, void*));

	OnBeginContact onBeginContact= nullptr;
	OnEndContact onEndContact= nullptr;
	OnPreSolveContact onPreSolveContact= nullptr; /// Return false if contact should be discarded
	OnPostSolveContact onPostSolveContact= nullptr;
	void* userData= nullptr; /// This is passed to callbacks
};

class Material;
class Fixture;
class ENGINE_API FixtureDef {
public:

	FixtureDef();
	FixtureDef(FixtureDef&& other);
	FixtureDef(const FixtureDef&);
	virtual ~FixtureDef();

	ContactFilter& getContactFilter(){ return contactFilter; }

	/// Takes pointer to the material
	virtual void setMaterial(const physics::Material& mat);
	const physics::Material* getMaterial() const { return material.get(); }

	/// Events sent by PhysMgr
	virtual void onEvent(global::Event&){}

	static const util::LinkedList<FixtureDef*>& getFixtureDefs(){ return fixtureDefs; }
protected:
	ContactFilter contactFilter;

private:
	friend class Fixture;

	util::CountedPointer<const physics::Material> material;

	static util::LinkedList<FixtureDef*> fixtureDefs;
	util::LinkedList<FixtureDef*>::Iter fixtureIt;

};

class InternalContactListener;

class ENGINE_API Fixture : public collision::Traceable, public util::Callbacker<util::OnDestroyCb> {
public:
	using Transform= collision::Traceable::Transform;
	static constexpr collision::Traceable::TypeId staticTraceableTypeId= collision::Traceable::Fixture;


	Fixture(const FixtureDef& def)
		: contactFilter(def.contactFilter)
		, material(def.material){
		addToTraceableTypeTree(collision::Traceable::Fixture);
	}
	Fixture(Fixture&&)= delete;
	Fixture(const Fixture&)= delete;
	virtual ~Fixture(){ util::OnDestroyCb::trigger(); }

	virtual Object& getObject() const = 0;

	FixtureCallbacks& getCallbacks(){ return callbacks; }

	const ContactFilter& getContactFilter() const { return contactFilter; }
	ContactFilter& getContactFilter(){ return contactFilter; }
	
	const physics::Material* getMaterial() const { return material.get(); }
	virtual const collision::Shape* getShape() const = 0;

protected:
	friend class InternalContactListener;

	ContactFilter contactFilter;
	FixtureCallbacks callbacks;
	util::CountedPointer<const physics::Material> material;

};

} // physics
} // clover

#endif // CLOVER_PHYSICS_FIXTURE_HPP
