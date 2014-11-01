#include "contact.hpp"
#include "fixture.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace physics {

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