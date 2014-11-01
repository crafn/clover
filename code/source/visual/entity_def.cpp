#include "entity_def.hpp"
#include "entitylogic.hpp"

namespace clover {
namespace visual {

EntityDef::EntityDef()
		: type(Type::None)
		, INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(offsetAttribute, "offset", util::SrtTransform3d()){
}

EntityDef::~EntityDef(){
}

EntityDef::Type EntityDef::getType() const {
	return type;
}

EntityLogic* EntityDef::createLogic() const {
	return new EntityLogic(*this);
}

void EntityDef::resourceUpdate(bool load, bool force){
	
	if (load || getResourceState() == State::Uninit){
		/// @todo
		setResourceState(State::Loaded);
	}
	else {
		setResourceState(State::Unloaded);
	} 
}

void EntityDef::createErrorResource(){
	setResourceState(State::Error);
}

} // visual
} // clover