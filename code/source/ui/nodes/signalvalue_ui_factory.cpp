#include "signalvalue_ui_factory.hpp"

namespace clover {
namespace ui { namespace nodes {

BaseSignalValueUi* SignalValueUiFactory::create(SignalType S){
	switch(S){
		#define SIGNAL(x, n) case SignalType::x: return new SignalValueUi<SignalType::x>;
		#include "nodes/signaltypes.def"
		#undef SIGNAL
		default:;
	}
	
	throw global::Exception("Invalid SignalType: %i", S);
}
	
}} // ui::nodes
} // clover