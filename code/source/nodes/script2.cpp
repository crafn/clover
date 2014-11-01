#include "script.hpp"
#include "animation/armaturepose.hpp"
#include "collision/baseshape_circle.hpp"
#include "collision/baseshape_polygon.hpp"
#include "compositionnodelogic.hpp"
#include "script/script_mgr.hpp"
#include "scriptnodeinstance.hpp"
#include "signaltypetraits.hpp"
#include "slotidentifier.hpp"
#include "util/unique_ptr.hpp"

namespace clover {
namespace nodes {

// Compiling this takes a lot of memory with gcc
void registerInstanceSlots_Part1(script::ScriptMgr& script){
	script.registerObjectType<BaseInputSlot>();

	#define SIGNAL(x,n) \
		script.registerObjectType<InputSlot<SignalType::x>>(); \
		script.registerMethod(&InputSlot<SignalType::x>::get, "get"); \
		script.registerMethod<InputSlot<SignalType::x>>(&InputSlot<SignalType::x>::setValueReceived, "setValueReceived"); \
		script.registerInheriting<BaseInputSlot, InputSlot<SignalType::x>>();
	#include "signaltypes.def"
	#undef SIGNAL

}

} // clover
} // nodes