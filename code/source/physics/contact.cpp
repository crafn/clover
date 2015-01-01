#include "box2d.hpp"
#include "contact.hpp"
#include "fixture.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace physics {

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
	manifold.normal= fromB2(wm.normal).normalized();
	manifold.averagePoint= fromB2(wm.points[0]);

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

template class BaseContact<ContactSide>;
template class BaseContact<PostSolveContactSide>;

template Contact BaseContact<ContactSide>::converted<Contact>() const;
template PostSolveContact BaseContact<ContactSide>::converted<PostSolveContact>() const;
template Contact BaseContact<PostSolveContactSide>::converted<Contact>() const;
template PostSolveContact BaseContact<PostSolveContactSide>::converted<PostSolveContact>() const;

ContactFilter::ContactFilter():
	box2dFilter(nullptr){

}

void ContactFilter::setGroup(Group g){
	ensure(box2dFilter);
	box2dFilter->categoryBits= (uint16)g;

	// Defaults
	switch (g){
		case Group::Generic: box2dFilter->maskBits= ~(uint16)DefaultIgnoreGroups::Generic; break;
		case Group::Character: box2dFilter->maskBits= ~(uint16)DefaultIgnoreGroups::Character; break;
		case Group::NoCollision: box2dFilter->maskBits= (uint16)~(uint16)DefaultIgnoreGroups::NoCollision; break;
		default: release_ensure(0 && "Not implemented"); break;
	}
}

void ContactFilter::setIgnoreGroups(Group g){
	ensure(box2dFilter);
	box2dFilter->maskBits= ~(uint16)g;
}

void ContactFilter::enableLocalOneWay(bool b){
	localOneWay.enabled= b;
}

void ContactFilter::setLocalOneWay(const util::Vec2d& allowed_direction, real32 min_allowed_dot_product){
	localOneWay.direction= allowed_direction;
	localOneWay.minDot= min_allowed_dot_product;
}

void ContactFilter::addIgnoreFixture(const Fixture& f){
	ensure(!hasIgnoreFixture(f));
	
	const Fixture* ptr= &f;
	ignoreFixtures.pushBack(ptr);
	
	fixtureDestroyListener.listen(f, [=] (){
		removeIgnoreFixture(*ptr);
	});
}

void ContactFilter::removeIgnoreFixture(const Fixture& f){
	fixtureDestroyListener.stopListening(f);
	ignoreFixtures.remove(&f);
}

} // physics
} // clover
