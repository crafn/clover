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

void registerInstanceSlots_Part2(script::ScriptMgr& script){
	script.registerObjectType<BaseOutputSlot>();

	#define SIGNAL(x,n) \
		script.registerObjectType<OutputSlot<SignalType::x>>(); \
		script.registerMethod( \
			static_cast<void (OutputSlot<SignalType::x>::*)(const SignalTypeTraits<SignalType::x>::Value&) const> \
				(&OutputSlot<SignalType::x>::send), "send"); \
		script.registerInheriting<BaseOutputSlot, OutputSlot<SignalType::x>>();
	#include "signaltypes.def"
	#undef SIGNAL

	// Trigger doesn't have to send a TriggerValue
	script.registerMethod(
		static_cast<void (OutputSlot<SignalType::Trigger>::*)() const>
			(&OutputSlot<SignalType::Trigger>::send), "send");

}

} // nodes
} // clover