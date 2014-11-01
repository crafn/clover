#include "armaturepose.hpp"
#include "script.hpp"
#include "script/script_mgr.hpp"

namespace clover {
namespace animation {

void registerToScript()
{
	auto&& script= *NONULL(script::gScriptMgr);

	typedef animation::ArmaturePose ArmaturePoseValue;
	script.registerObjectType<ArmaturePoseValue>();
}

} // animation
} // clover
