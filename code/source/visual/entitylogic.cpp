#include "entitylogic.hpp"

namespace clover {
namespace visual {
	
EntityLogic::EntityLogic(const EntityDef& def)
		: definition(&def)
		, layer(Layer::World)
		, coordinateSpace(util::Coord::World)
		, scaleCoordinateSpace(util::Coord::None){
}

EntityLogic::~EntityLogic(){
}

const EntityDef& EntityLogic::getDef() const {
	ensure(definition);
	return *definition;
}

void EntityLogic::apply(const EntityLogic& other){
	transform= other.transform;
	layer= other.layer;
}

void EntityLogic::setDef(const EntityDef& def){
	definition= &def;
}

} // visual
} // clover