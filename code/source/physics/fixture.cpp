#include "fixture.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace physics {

util::LinkedList<FixtureDef*> FixtureDef::fixtureDefs;

FixtureDef::FixtureDef(){
	fixtureDefs.pushBack(this);
	fixtureIt= --fixtureDefs.end();
}

FixtureDef::FixtureDef(FixtureDef&& other):
	material(std::move(other.material)){

	fixtureIt= other.fixtureIt;
	*fixtureIt= this;
	other.fixtureIt= fixtureDefs.end();
}

FixtureDef::FixtureDef(const FixtureDef& other):
	material(other.material){

	fixtureDefs.pushBack(this);
	fixtureIt= --fixtureDefs.end();
}

FixtureDef::~FixtureDef(){
	if (fixtureIt != fixtureDefs.end()){
		fixtureDefs.erase(fixtureIt);
	}
}

void FixtureDef::setMaterial(const physics::Material& mat){
	material= &mat;
}

} // physics
} // clover