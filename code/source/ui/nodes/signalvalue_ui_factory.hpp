#ifndef CLOVER_SIGNALVALUE_UI_FACTORY_HPP
#define CLOVER_SIGNALVALUE_UI_FACTORY_HPP

#include "build.hpp"
#include "signalvalue_ui.hpp"

namespace clover {
namespace ui { namespace nodes {

class SignalValueUiFactory {
public:
	static BaseSignalValueUi* create(SignalType S);
};

}} // ui::nodes
} // clover

#endif // CLOVER_SIGNALVALUE_UI_FACTORY_HPP