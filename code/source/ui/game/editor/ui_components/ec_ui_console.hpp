#ifndef CLOVER_UI_EDITOR_EC_UI_CONSOLE_HPP
#define CLOVER_UI_EDITOR_EC_UI_CONSOLE_HPP

#include "../editorcomponent_ui.hpp"
#include "build.hpp"
#include "game/editor/components/ec_console.hpp"
#include "gui/element_checkbox.hpp"
#include "gui/element_floating.hpp"
#include "gui/element_layout_grid.hpp"
#include "gui/element_layout_linear.hpp"
#include "gui/element_slider.hpp"
#include "gui/element_textlabel.hpp"
#include "gui/element_textfield.hpp"
#include "util/deque.hpp"

namespace clover {
namespace ui { namespace game { namespace editor {

template <>
struct EditorComponentUiTraits<ConsoleEc> {
	static util::Coord defaultRadius(){ return util::Coord::VSt({1.0,0.4}); }
};

class ConsoleEcUi : public EditorComponentUi<ConsoleEc> {
public:
	typedef EditorComponentUi<ConsoleEc> BaseClass;
	
	ConsoleEcUi(ConsoleEc&);
	virtual ~ConsoleEcUi();
	
	virtual void onEvent(global::Event&);
	virtual void update();
	
protected:
	virtual void onResize();
	
private:
	void append(const debug::DebugPrint::Buffer& b);
	void updateLineLabels();
	
	void readValues();
	
	struct Line {
		util::Color color;
		util::Str8 string;
	};
	
	static constexpr real64 controlLayoutHeight= 0.03;
	
	util::Deque<Line> lines;
	util::DynArray<gui::LinearLayoutElement> lineLayouts;
	util::DynArray<gui::TextLabelElement> lineLabels;
	
	gui::LinearLayoutElement contentLayout;
	
	// Contains buttons and fields
	gui::LinearLayoutElement controlLayout;
	gui::ButtonElement filterButton;
	gui::TextFieldElement commandField;
	
	
	// Filter panel
	util::Coord filterPanelRadius() const { return util::Coord::VF(0.4); }
	
	gui::FloatingElement filterFloating;
		gui::PanelElement filterPanel;
		
		gui::LinearLayoutElement filterPanelLayout;
		// Verbosity for debugPrint
		gui::TextLabelElement verbositySliderLabel;
		gui::SliderElement verbositySlider;
		
		// Flags
		gui::TextLabelElement filterLabel;
		static constexpr uint32 filterCheckCount= static_cast<uint32>(debug::Ch::Last);
		gui::GridLayoutElement filterLayout;
		util::DynArray<gui::CheckBoxElement> filterChecks;
};

}}} // ui::game::editor
} // clover

#endif // CLOVER_UI_EDITOR_EC_UI_CONSOLE_HPP