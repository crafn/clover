#include "contactlistener.hpp"
#include "debug/print.hpp"
#include "ghostlydynamics.hpp"
#include "physics/b2_fixtureuserdata.hpp"
#include "physics/fixture.hpp"

namespace clover {
namespace physics {

template <typename T>
bool isBreakingContact(const T& c){
	/// @todo Add normal breaking
	return	c.getSide(0).object->isPartiallyBreakable() ||
			c.getSide(1).object->isPartiallyBreakable();
}

template <typename T, typename... Args>
void tryInvokeCallback(const std::function<T>& cb, Args... args){
	if (!cb)
		return;
	
	try {
		cb(args...);
	}
	catch (global::Exception& e){
		print(debug::Ch::Phys, debug::Vb::Critical, "Contact callback failed: %s", e.what());
	}
}

void tryInvokePreSolveCallback(const std::function<bool (const Contact&)>& cb, const Contact& c, b2Contact* b2_c){
	if (!cb)
		return;
	
	try {
		if (!cb(c))
			b2_c->SetEnabled(false);
	}
	catch (global::Exception& e){
		print(debug::Ch::Phys, debug::Vb::Critical, "OnPreSolveCallback callback failed: %s", e.what());
	}
}

BaseContactListener::BaseContactListener(){
	InternalContactListener::onContactListenerCreate(*this);
}

BaseContactListener::~BaseContactListener(){
	InternalContactListener::onContactListenerDestroy(*this);
}


void InternalContactListener::onContactListenerCreate(BaseContactListener& l){
	subContactListeners.pushBack(&l);
}

void InternalContactListener::onContactListenerDestroy(BaseContactListener& l){
	auto it= subContactListeners.find(&l);
	ensure(it != subContactListeners.end());
	subContactListeners.erase(it);
}

util::DynArray<Contact> InternalContactListener::getGhostlyContacts() const {
	util::DynArray<Contact> contacts;
	contacts.reserve(ghostlyRecords.size());
	for (const auto& pair : ghostlyRecords){
		contacts.emplaceBack(*NONULL(pair.first), pair.second->isNew);
	}
	return contacts;
}

util::DynArray<PostSolveContact> InternalContactListener::popBreakingContacts(){
	auto ret= breakingContacts;
	breakingContacts.clear();
	return ret;
}

void InternalContactListener::BeginContact(b2Contact* box2d_contact){
	ContactRecord& record= contactRecords[box2d_contact];

	Contact c(*box2d_contact, record.isNew);

	// Fixture checks needed for sensors
	bool contact_ignored=
		record.preventCollision ||
		c.getSide(0).fixture->getContactFilter().
			hasIgnoreFixture(*c.getSide(1).fixture) ||
		c.getSide(1).fixture->getContactFilter().
			hasIgnoreFixture(*c.getSide(0).fixture);

	if (contact_ignored){
		// Needed because sensors aren't PreSolved
		record.preventCollision= true;
	}
	else {
		if (isGhostlyContact(c)){
			ghostlyRecords[box2d_contact]= &record;
			
			PostSolveContact post_c= onGhostlyContact(c);
			if (isBreakingContact(c))
				breakingContacts.pushBack(post_c);
		}

		tryInvokeCallback(c.getSide(0).fixture->callbacks.onBeginContact, c);
		tryInvokeCallback(c.getSide(1).fixture->callbacks.onBeginContact, c.switchedSides());

		for (auto& m : subContactListeners)
			m->onBeginContact(c);
	}
}

void InternalContactListener::EndContact(b2Contact* box2d_contact){
	auto it= contactRecords.find(box2d_contact);
	ensure(it != contactRecords.end());

	Contact c(*box2d_contact, it->second.isNew);

	bool contact_ignored= it->second.preventCollision;
	contactRecords.erase(it);

	auto ghost_it= ghostlyRecords.find(box2d_contact);
	if (ghost_it != ghostlyRecords.end())
		ghostlyRecords.erase(ghost_it);

	if (!contact_ignored){
		tryInvokeCallback(c.getSide(0).fixture->callbacks.onEndContact, c);
		tryInvokeCallback(c.getSide(1).fixture->callbacks.onEndContact, c.switchedSides());

		for (auto& m : subContactListeners)
			m->onEndContact(c);
	}
}

void InternalContactListener::PreSolve(b2Contact* box2d_contact, const b2Manifold* old_manifold){

	ContactRecord& record= contactRecords[box2d_contact];
	Contact c(*box2d_contact, record.isNew);
	
	if (record.isNew){
		
		if (c.getSide(0).fixture->getContactFilter().hasIgnoreFixture(*c.getSide(1).fixture))
			record.preventCollision= true;
		else if (c.getSide(1).fixture->getContactFilter().hasIgnoreFixture(*c.getSide(0).fixture))
			record.preventCollision= true;
		else {
			const auto& local_one_way_a= c.getSide(0).fixture->getContactFilter().getLocalOneWayData();
			const auto& local_one_way_b= c.getSide(1).fixture->getContactFilter().getLocalOneWayData();

			if (local_one_way_a.enabled || local_one_way_b.enabled){
				// One-way platforms

				ContactManifold m= c.getManifold();

				util::Vec2d from_a_to_b= c.getSide(1).object->getPosition() - c.getSide(0).object->getPosition();
				util::Vec2d one_way_vec_a= local_one_way_a.direction.rotated(c.getSide(0).object->getRotation());
				util::Vec2d one_way_vec_b= local_one_way_b.direction.rotated(c.getSide(1).object->getRotation());


				if (!c.getSide(1).object->isStatic()
					&& local_one_way_a.enabled
					&& from_a_to_b.normalized().dot(one_way_vec_a) < -local_one_way_a.minDot)
						record.preventCollision= true;

				if (!c.getSide(0).object->isStatic()
					&& local_one_way_b.enabled
					&& from_a_to_b.normalized().dot(one_way_vec_b) > local_one_way_b.minDot)
						record.preventCollision= true;
			}
		}

		if (ghostlyRecords.find(box2d_contact) != ghostlyRecords.end())
			box2d_contact->SetEnabled(false); // Manual collision resolving
	}

	if (record.preventCollision){
		box2d_contact->SetEnabled(false);
		return;
	}

	// Callbacks
	
	tryInvokePreSolveCallback(c.getSide(0).fixture->callbacks.onPreSolveContact, c, box2d_contact);
	tryInvokePreSolveCallback(c.getSide(1).fixture->callbacks.onPreSolveContact, c.switchedSides(), box2d_contact);

	if (!box2d_contact->IsEnabled())
		return;

	for (auto& m : subContactListeners)
		m->onPreSolveContact(c);

}

void InternalContactListener::PostSolve(b2Contact* box2d_contact, const b2ContactImpulse* impulse){
	ContactRecord& record= contactRecords[box2d_contact];

	if (!record.preventCollision){
		PostSolveContact c(*box2d_contact, record.isNew);

		util::Vec2d normal= c.getManifold().normal;
		/// @todo Make sure this point in right direction
		util::Vec2d tangent= c.getManifold().normal.normal();

		for (int32 i=0; i<impulse->count; ++i){
			c.getSide(0).totalImpulse += normal*impulse->normalImpulses[i];
			c.getSide(1).totalImpulse += -normal*impulse->normalImpulses[i];
			c.getSide(0).totalImpulse += tangent*impulse->tangentImpulses[i];
			c.getSide(1).totalImpulse += -tangent*impulse->tangentImpulses[i];
		}

		if (isBreakingContact(c)){
			breakingContacts.pushBack(c);
		}

		tryInvokeCallback(c.getSide(0).fixture->callbacks.onPostSolveContact, c);
		tryInvokeCallback(c.getSide(1).fixture->callbacks.onPostSolveContact, c.switchedSides());

		for (auto& m : subContactListeners)
			m->onPostSolveContact(c);
	}

	record.isNew= false;
}


util::DynArray<BaseContactListener*> InternalContactListener::subContactListeners;

} // physics
} // clover
