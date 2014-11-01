#ifndef CLOVER_UI_NODES_SIGNALVALUE_UI_HPP
#define CLOVER_UI_NODES_SIGNALVALUE_UI_HPP

#include "build.hpp"
#include "basesignalvalue_ui.hpp"
#include "gui/element_floating.hpp"
#include "gui/element_panel.hpp"
#include "signalvalue_ui_traits.hpp"
// .tpp
#include "ui/userinput.hpp"
#include "hardware/mouse.hpp"
#include "hardware/keyboard.hpp"

namespace clover {
namespace ui { namespace nodes {

template <SignalType S>
class SignalValueUi : public BaseSignalValueUi {
public:
	typedef typename SignalValueUiTraits<S>::Value Value;
	SignalValueUi();
	
	virtual ~SignalValueUi(){}
	
	virtual gui::Element& getViewGuiElement(){ return viewElement; }
	virtual void showEditControls(bool b);
	virtual bool isEditControlsShowing() const { return floatingElement.isActive(); }
	
	virtual void setValue(const boost::any& v, bool only_edit= false);
	virtual boost::any getValue() const { return SignalValueUiTraits<S>::valueFromEdit(editElement); }

private:
	typename SignalValueUiTraits<S>::ViewElement viewElement;
	typename SignalValueUiTraits<S>::EditElement editElement;
	gui::FloatingElement floatingElement;
	gui::PanelElement bgPanel;
};

#include "signalvalue_ui.tpp"

}} // ui::nodes
} // clover

#endif // CLOVER_UI_NODES_SIGNALVALUE_UI_HPP