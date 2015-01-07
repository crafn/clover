#ifndef CLOVER_UI_NODES_BASESIGNALVALUE_UI_HPP
#define CLOVER_UI_NODES_BASESIGNALVALUE_UI_HPP

#include "build.hpp"
#include "gui/element.hpp"

namespace clover {
namespace ui { namespace nodes {

class BaseSignalValueUi {
public:
	virtual ~BaseSignalValueUi(){}
	
	virtual gui::Element& getViewGuiElement()= 0;
	virtual void showEditControls(bool b)= 0;
	virtual bool isEditControlsShowing() const = 0;
	
	virtual void setValue(const util::Any& v, bool only_edit= false) = 0;
	virtual util::Any getValue() const = 0;
	
	typedef std::function<void()> Callback;
	void setOnValueModifyCallback(const Callback& c){ onValueModify= c; }
	void setOnValueUnsetCallback(const Callback& c){ onValueUnset= c; }
	
protected:

	Callback onValueModify;
	Callback onValueUnset;
};

}} // ui::nodes
} // clover

#endif // CLOVER_UI_NODES_BASESIGNALVALUE_UI_HPP