#ifndef CLOVER_UI_EDITOR_EC_UI_PHYSICSTOOLS_HPP
#define CLOVER_UI_EDITOR_EC_UI_PHYSICSTOOLS_HPP

#include "../editorcomponent_ui.hpp"
#include "build.hpp"
#include "game/editor/components/ec_physicstools.hpp"
#include "gui/element_checkbox.hpp"
#include "gui/element_layout_linear.hpp"
#include "gui/element_listbox.hpp"
#include "gui/element_slider.hpp"
#include "gui/element_textfield.hpp"

namespace clover {
namespace ui { namespace game { namespace editor {

template <>
struct EditorComponentUiTraits<PhysicsToolsEc> {
	static util::Coord defaultRadius(){ return util::Coord::VF({0.65, 0.35}); }
};

class PhysicsToolsEcUi : public EditorComponentUi<PhysicsToolsEc> {
public:
	typedef EditorComponentUi<PhysicsToolsEc> BaseType;
	
	PhysicsToolsEcUi(PhysicsToolsEc& comp);
	virtual ~PhysicsToolsEcUi(){}

	virtual void update() override;

protected:
	virtual void onResize() override;

private:
	void readValues();

	gui::LinearLayoutElement contentLayout;
		
	gui::LinearLayoutElement primaryLayout;
		gui::TextLabelElement primaryTitleLabel;
		gui::TextLabelElement primaryGrabLabel;
	gui::LinearLayoutElement secondaryLayout;
		gui::TextLabelElement secondaryTitleLabel;
		gui::TextLabelElement secondaryActionLabel;
	
	gui::LinearLayoutElement drawLayout;
	
		gui::LinearLayoutElement drawCheckLayout; // Layout for checkbox
		gui::CheckBoxElement drawCheck;
		
		static constexpr uint32 drawFlagCheckCount= physics::Draw::flagCount;
		gui::LinearLayoutElement drawFlagCheckLayout[drawFlagCheckCount]; // Layouts for flag-checkboxes
		gui::CheckBoxElement drawFlagCheck[drawFlagCheckCount]; // physics::Draw flags
		
		gui::TextLabelElement drawAlphaSliderLabel;
		gui::SliderElement drawAlphaSlider;
};

}}} // ui::game::editor
} // clover

#endif // CLOVER_UI_EDITOR_EC_UI_PHYSICSTOOLS_HPP