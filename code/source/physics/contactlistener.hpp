#ifndef CLOVER_PHYSICS_CONTACTLISTENER_HPP
#define CLOVER_PHYSICS_CONTACTLISTENER_HPP

#include "build.hpp"
#include "util/map.hpp"
#include "util/dyn_array.hpp"
#include "physics/contact.hpp"

#include <Box2D/Box2D.h>

namespace clover {
namespace physics {

/// Inherit from this if you need contact information
class BaseContactListener {
public:
	BaseContactListener();
	virtual ~BaseContactListener();

	/// Dunno if these should be called before or after fixture-callbacks. Currently called after
	virtual void onBeginContact(const Contact&)= 0;
	virtual void onEndContact(const Contact&)= 0;
	virtual void onPreSolveContact(const Contact&)= 0;
	virtual void onPostSolveContact(const PostSolveContact&)= 0;

private:
};


/// Don't use this outside physics system
class InternalContactListener : public b2ContactListener {
public:
	/// BaseContactListener calls automatically
	static void onContactListenerCreate(BaseContactListener&);
	static void onContactListenerDestroy(BaseContactListener&);

	util::DynArray<Contact> getGhostlyContacts() const;
	util::DynArray<PostSolveContact> popBreakingContacts();

	// Comments taken from Box2D manual

	/// This is called when two fixtures begin to overlap. This is called for sensors and non-sensors. This event can only occur inside the time step.
	void BeginContact(b2Contact* contact);

	/// This is called when two fixtures cease to overlap. This is called for sensors and non-sensors. This may be called when a body is destroyed, so this event can occur outside the time step.
	void EndContact(b2Contact* contact);

	/// This is called after collision detection, but before collision resolution. This gives you a chance to disable the contact based on the current configuration.
	void PreSolve(b2Contact* contact, const b2Manifold* old_manifold);

	/// The post solve event is where you can gather collision impulse results. If you don’t care about the impulses, you should probably just implement the pre-solve event.
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);

private:
	struct ContactRecord {
		bool isNew= true;
		bool preventCollision= false; // Don't collide / inform contact
	};

	util::Map<b2Contact*, ContactRecord> contactRecords;
	util::Map<b2Contact*, ContactRecord*> ghostlyRecords;
	util::DynArray<PostSolveContact> breakingContacts;

	static util::DynArray<BaseContactListener*> subContactListeners;
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_CONTACTLISTENER_HPP
