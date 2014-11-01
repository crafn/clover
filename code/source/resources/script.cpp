#include "script.hpp"
#include "cache.hpp"
#include "game/physicalmaterial.hpp"
#include "script/script_mgr.hpp"

namespace clover {
namespace resources {

void registerToScript(){
	/// @todo Automatic registering for every resource type
	script::gScriptMgr->registerObjectType<Cache>();
	script::gScriptMgr->registerObjectType<game::PhysicalMaterial>();
	script::gScriptMgr->registerInheriting<physics::Material, game::PhysicalMaterial>();
	script::gScriptMgr->registerMethod(&Cache::getResource<game::PhysicalMaterial>, "getPhysicalMaterialResource");
	script::gScriptMgr->registerGlobalProperty(gCache, "gCache");
}

} // resources
} // clover