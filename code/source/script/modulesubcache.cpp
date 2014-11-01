#include "modulesubcache.hpp"
#include "script_mgr.hpp"
#include "nodes/nodetype.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace script {

ModuleSubCache::~ModuleSubCache(){
}

void ModuleSubCache::preLoad(){
	BaseType::preLoad();
}

void ModuleSubCache::registerEverything(){
}

} // script
} // clover