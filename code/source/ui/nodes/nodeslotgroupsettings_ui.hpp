#ifndef CLOVER_UI_NODES_NODESLOTGROUPSETTINGS_UI_HPP
#define CLOVER_UI_NODES_NODESLOTGROUPSETTINGS_UI_HPP

#include "build.hpp"
#include "createnodeslotdialog.hpp"
#include "gui/element_checkbox.hpp"
#include "gui/element_floating.hpp"
#include "gui/element_layout_linear.hpp"
#include "gui/element_panel.hpp"
#include "nodes/compositionnodelogic.hpp"
#include "util/unique_ptr.hpp"

namespace clover {
namespace ui { namespace nodes {

/// A panel in which dynamic slots are added/removed
class NodeSlotGroupSettingsUi {
public:
	NodeSlotGroupSettingsUi(CompositionNodeLogic& comp_node);
	NodeSlotGroupSettingsUi(NodeSlotGroupSettingsUi&&);
	NodeSlotGroupSettingsUi(const NodeSlotGroupSettingsUi&)= delete;
	
	void show(bool true_to_show, bool is_input);
	void setPosition(const util::Coord& pos);
	
	typedef std::function<void()> CallbackType;
	
	void setOnPreSlotModifyCallback(CallbackType c){ onPreSlotModify= c; }
	void setOnPostSlotModifyCallback(CallbackType c){ onPostSlotModify= c; }
	
private:
	
	void addCheckBox(bool active, const SlotTemplate& slot);
	void addVariantAddButton(CompositionNodeSlotTemplateGroup& group);
	void addGroupGuiElements(CompositionNodeSlotTemplateGroup& group);
	void showCreateSlotDialog(CompositionNodeSlotTemplateGroup& group);
	
	void bindCallbacks();
	
	struct CheckBoxWrap {
		gui::CheckBoxElement element;
		gui::TextLabelElement signalTypeLabel;
		SlotTemplate slotTemplate;
	};
	
	struct VariantAddButtonWrap {
		gui::ButtonElement element;
		CompositionNodeSlotTemplateGroup* group;
	};
	
	CompositionNodeLogic* compositionNodeLogic;
	
	gui::FloatingElement floating;
	gui::PanelElement bgPanel;
	gui::LinearLayoutElement layout;
	
	CallbackType onPreSlotModify;
	CallbackType onPostSlotModify;
	
	/// Is currently showing input or output slots
	bool isInput;
	
	util::DynArray<CheckBoxWrap> slotCheckBoxes;
	util::DynArray<VariantAddButtonWrap> variantAddButtons;
	
	util::UniquePtr<CreateNodeSlotDialog> createSlotDialog;
};

}} // ui::nodes
} // clover

#endif // CLOVER_UI_NODES_NODESLOTGROUPSETTINGS_UI_HPP