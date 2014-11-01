#ifndef CLOVER_PHYSICS_FIXTURE_HPP
#define CLOVER_PHYSICS_FIXTURE_HPP

#include "build.hpp"
#include "collision/traceable.hpp"
#include "global/eventreceiver.hpp"
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
	std::function<void (const Contact&)> onBeginContact;
	std::function<void (const Contact&)> onEndContact;
	std::function<bool (const Contact&)> onPreSolveContact; /// Return false if contact should be discarded
	std::function<void (const PostSolveContact&)> onPostSolveContact;
};

class Material;
class Fixture;
class FixtureDef {
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

class Fixture : public collision::Traceable, public util::Callbacker<util::OnDestroyCb> {
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
namespace util {

template <>
struct TypeStringTraits<physics::FixtureCallbacks> {
	static util::Str8 type(){ return "physics::FixtureCallbacks"; }
};

template <>
struct TypeStringTraits<physics::Fixture> {
	static util::Str8 type(){ return "physics::Fixture"; }
};

} // util
} // clover

#endif // CLOVER_PHYSICS_FIXTURE_HPP
