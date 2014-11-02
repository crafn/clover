#ifndef CLOVER_PHYSICS_CONTACT_HPP
#define CLOVER_PHYSICS_CONTACT_HPP

#include "b2_fixtureuserdata.hpp"
#include "build.hpp"
#include "script/typestring.hpp"
#include "util/cb_listener.hpp"
#include "util/dyn_array.hpp"
#include "util/vector.hpp"

#include <Box2D/Box2D.h>

namespace clover {
namespace physics {

class Object;
class Fixture;

struct ContactManifold {
	util::Vec2d normal; // From surface of the first fixture
	util::Vec2d averagePoint; // Can be used in many situations instead of accurate contact points. In world coordinates

	// Box2d provides also states for contact points if needed someday
	//util::DynArray<util::Vec2d> points; // Contact points
};

struct ContactSide {
	Fixture* fixture= nullptr;
	Object* object= nullptr;
};

struct PostSolveContactSide : public ContactSide {
	util::Vec2d totalImpulse;
};

template <typename T>
class BaseContact {
public:
	BaseContact()= default;
	BaseContact(const b2Contact& c, bool is_new);

	/// @todo Friction, restitution

	ContactManifold getManifold(uint32 i= 0) const;
	const T& getSide(uint32 i) const;
	T& getSide(uint32 i);

	bool isNew() const { return !old; }

	/// @return BaseContact<T> with sides switched
	BaseContact<T> switchedSides() const;

	template <typename U>
	U converted() const;

private:
	template <typename U> friend class BaseContact;
	ContactManifold manifold;
	bool old;
	bool switched;
	T sides[2];
};

typedef BaseContact<ContactSide> Contact;
typedef BaseContact<PostSolveContactSide> PostSolveContact;



class ContactFilter {
public:
	ContactFilter();


	enum class Group : uint16 {
		Generic=	1<<0,
		Character=	1<<1, // A bit poor place to define gameplay stuff
		NoCollision=1<<2
	};


	struct OneWayData {
		OneWayData():enabled(false), minDot(0){}

		bool enabled;
		util::Vec2d direction;
		real32 minDot;
	};

	/// Sets collision-group and default ignoregroups
	void setGroup(Group g);
	void setIgnoreGroups(Group g);

	void enableLocalOneWay(bool b=true);
	void setLocalOneWay(const util::Vec2d& allowed_direction, real32 min_allowed_dot_product= 0.0);

	const OneWayData& getLocalOneWayData() const { return localOneWay; }

	void addIgnoreFixture(const Fixture& f);
	void removeIgnoreFixture(const Fixture& f);

	bool hasIgnoreFixture(const Fixture& f) const { return ignoreFixtures.find(&f) != ignoreFixtures.end(); }

	void setB2Filter(b2Filter& f){ box2dFilter= &f; }
private:

	enum class DefaultIgnoreGroups : uint16 {
		Generic= 0,
		Character= (uint16)Group::Character,
		NoCollision= (uint16)-1
	};


	OneWayData localOneWay;
	util::DynArray<const Fixture*> ignoreFixtures;

	util::CbMultiListener<util::OnDestroyCb> fixtureDestroyListener;

	b2Filter* box2dFilter;
};

template <typename T>
BaseContact<T>::BaseContact(const b2Contact& b2_contact, bool is_new):
	old(!is_new), switched(false){
	auto fix_a= b2_contact.GetFixtureA();
	auto fix_b= b2_contact.GetFixtureB();

	auto user_data_a= (B2FixtureUserData*)fix_a->GetUserData();
	auto user_data_b= (B2FixtureUserData*)fix_b->GetUserData();

	ensure(user_data_a && user_data_b);

	sides[0].fixture= user_data_a->owner;
	sides[1].fixture= user_data_b->owner;

	sides[0].object= &sides[0].fixture->getObject();
	sides[1].object= &sides[1].fixture->getObject();

	b2WorldManifold wm;
	b2_contact.GetWorldManifold(&wm);
	manifold.normal= util::Vec2d(wm.normal).normalized();
	manifold.averagePoint= wm.points[0];

	// No manifold for sensors (Box2d restriction)
	if (	b2_contact.GetFixtureA()->IsSensor() ||
			b2_contact.GetFixtureB()->IsSensor())
		manifold.normal= util::Vec2d{};
}

template <typename T>
ContactManifold BaseContact<T>::getManifold(uint32 i) const {
	ensure_msg(	manifold.normal.lengthSqr() > 0.0,
				"Manifolds aren't generated for sensor collisions");
	return i == 0 ? manifold : switchedSides().manifold;
}

template <typename T>
const T& BaseContact<T>::getSide(uint32 i) const {
	ensure(i<2);
	return sides[i];
}

template <typename T>
T& BaseContact<T>::getSide(uint32 i) {
	ensure(i<2);
	return sides[i];
}

template <typename T>
BaseContact<T> BaseContact<T>::switchedSides() const {
	BaseContact<T> ret(*this);
	std::swap(ret.sides[0], ret.sides[1]);
	ret.manifold.normal *= -1.0;
	ret.switched= !ret.switched;
	return (ret);
}

template <typename T>
template <typename U>
U BaseContact<T>::converted() const {
	U ret;
	ret.manifold= manifold;
	ret.old= old;
	ret.switched= switched;
	static_assert(!std::is_same<U, T>::value, "todo proper side converting");
	*(static_cast<ContactSide*>(&ret.sides[0]))= sides[0];
	*(static_cast<ContactSide*>(&ret.sides[1]))= sides[1];
	return ret;
}

} // physics
namespace util {

template <>
struct TypeStringTraits<physics::Contact> {
	static util::Str8 type(){ return "physics::Contact"; }
};

template <>
struct TypeStringTraits<physics::ContactSide> {
	static util::Str8 type(){ return "physics::ContactSide"; }
};

template <>
struct TypeStringTraits<physics::PostSolveContact> {
	static util::Str8 type(){ return "physics::PostSolveContact"; }
};

template <>
struct TypeStringTraits<physics::PostSolveContactSide> {
	static util::Str8 type(){ return "physics::ContactSide"; }
};

} // util
} // clover

#endif // CLOVER_PHYSICS_CONTACT_HPP