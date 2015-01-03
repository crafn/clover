#include "entity_def_model.hpp"
#include "global/env.hpp"
#include "resources/cache.hpp"
#include "subcache_entity.hpp"

namespace clover {
namespace resources {

util::HashMap<util::Str8, visual::EntityDef*> VisualEntityDefSubCache::allEntityDefs;

VisualEntityDefSubCache::~VisualEntityDefSubCache(){
}

visual::EntityDef* VisualEntityDefSubCache::findResource(const util::Str8& identifier){
	auto it= allEntityDefs.find(identifier);
	if (it == allEntityDefs.end()) return nullptr;
	return it->second;
}

const visual::EntityDef& VisualEntityDefSubCache::getErrorResource(){
	return global::g_env->resCache->getErrorResource<visual::ModelEntityDef>();
}

void VisualEntityDefSubCache::addEntityDef(const util::Str8& identifier, visual::EntityDef& entitydef){
	auto it= allEntityDefs.find(identifier);
	if (it != allEntityDefs.end() && it->second->getType() != visual::EntityDef::Type::None) // Override plain EntityDefs
		throw ResourceException("EntityDef identifier already in use: %s", identifier.cStr());
	
	allEntityDefs[identifier]= &entitydef;
}

visual::EntityDef& VisualEntityDefSubCache::createNewResource(const util::Str8& identifier){
	// It's possible that obsolete VisualEntityDef exists and creating resource should override it
	visual::EntityDef& res= BaseType::createNewResource(identifier);
	addEntityDef(identifier, res);
	return res;
}

} // resources
} // clover
