#ifndef CLOVER_UI_NODES_CREATENODESLOTDIALOG_HPP
#define CLOVER_UI_NODES_CREATENODESLOTDIALOG_HPP

#include "build.hpp"
#include "gui/element_button.hpp"
#include "gui/element_combobox.hpp"
#include "gui/element_floating.hpp"
#include "gui/element_layout_linear.hpp"
#include "gui/element_panel.hpp"
#include "gui/element_textfield.hpp"
#include "nodes/signaltype.hpp"

#include <functional>

namespace clover {
namespace ui { namespace nodes {
using namespace clover::nodes;

class CreateNodeSlotDialog {
public:
	CreateNodeSlotDialog();
	
	using Callback= std::function<void (SignalType, util::Str8)>;
	/// Close callback is called when dialog is closed (SignalType == None if cancelled)
	void setOnCloseCallback(Callback cb);
	
private:
	Callback onClose;

	gui::FloatingElement floating;
	gui::PanelElement bgPanel;
	gui::LinearLayoutElement layout;
	gui::ComboBoxElement typeCombo;
	gui::TextFieldElement nameField;
	gui::ButtonElement createButton;
};

}} // ui::nodes
} // clover

#endif // CLOVER_UI_NODES_CREATENODESLOTDIALOG_HPP