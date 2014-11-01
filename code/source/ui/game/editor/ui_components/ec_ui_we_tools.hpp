#ifndef CLOVER_UI_EDITOR_EC_UI_WE_TOOLS_HPP
#define CLOVER_UI_EDITOR_EC_UI_WE_TOOLS_HPP

#include "../editorcomponent_ui.hpp"
#include "build.hpp"
#include "game/editor/components/ec_we_tools.hpp"
#include "gui/element_combobox.hpp"
#include "gui/element_checkbox.hpp"
#include "ui/hid/actionlistener.hpp"

/// @todo Replace with util
#include <boost/optional.hpp>

namespace clover {
namespace ui { namespace game { namespace editor {

template <>
struct EditorComponentUiTraits<WeToolsEc> {
	static util::Coord defaultRadius(){ return util::Coord::VF({0.65, 0.2}); }
};

class WeToolsEcUi : public EditorComponentUi<WeToolsEc> {
public:
	typedef EditorComponentUi<WeToolsEc> BaseType;
	
	WeToolsEcUi(WeToolsEc& comp);
	virtual ~WeToolsEcUi(){}
	
	virtual void update() override;
	
protected:
	virtual void onResize() override;

private:
	void readValues();
	
	gui::LinearLayoutElement contentLayout;
		
	gui::LinearLayoutElement spawnLayout;
		gui::TextLabelElement spawnTitleLabel;
		gui::ComboBoxElement spawnCombo;
		gui::ButtonElement spawnButton;
		
	gui::LinearLayoutElement debugLayout;
		gui::TextLabelElement debugTitleLabel;
		gui::LinearLayoutElement debugDrawCheckLayout; // Layout for checkbox
		gui::CheckBoxElement debugDrawCheck;
		gui::CheckBoxElement chunksLockedCheck;
	
	boost::optional<ui::hid::ActionListener<>> terrainEraseListener;
	boost::optional<ui::hid::ActionListener<>> weDeleteListener;
};

}}} // ui::game::editor
} // clover

#endif // CLOVER_UI_EDITOR_EC_UI_WE_TOOLS_HPP