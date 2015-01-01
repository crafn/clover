#ifndef CLOVER_PHYSICS_CONTACT_HPP
#define CLOVER_PHYSICS_CONTACT_HPP

#include "b2_fixtureuserdata.hpp"
#include "build.hpp"
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
} // physics
} // clover

#endif // CLOVER_PHYSICS_CONTACT_HPP
