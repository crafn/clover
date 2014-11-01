#ifndef CLOVER_UI_EDITOR_EDITORVIEW_UI_HPP
#define CLOVER_UI_EDITOR_EDITORVIEW_UI_HPP

#include "build.hpp"
#include "common.hpp"
#include "global/eventreceiver.hpp"
#include "game/editor/editorview.hpp"
#include "baseeditorcomponent_ui.hpp"
#include "gui/element_layout_linear.hpp"
#include "gui/element_combobox.hpp"

#include <memory>

namespace clover {
namespace ui { namespace game { namespace editor {

/// Handles user interface of a single EditorView
class EditorViewUi : public global::EventReceiver {
public:
	EditorViewUi(EditorView& view);
	EditorViewUi(EditorViewUi&&)= delete;
	EditorViewUi& operator=(EditorViewUi&&)= delete;
	virtual ~EditorViewUi();

	virtual void onEvent(global::Event&);

	void update();

	const EditorView& getView() const { return *view; }

	void setActive(bool b=true);
	void toggle();
	bool isActive() const { return active; }
	
private:
	static constexpr real64 addButtonHorizontalRad= 0.19;
	static constexpr real64 addButtonVerticalRad= 0.10;
	static util::Coord addButtonRad(){ return util::Coord::VF({addButtonHorizontalRad, addButtonVerticalRad}); }

	BaseEditorComponentUi& createComponent(const util::Str8& name, int32 row);
	void destroyComponent(BaseEditorComponentUi&);
	util::Coord calcRowEmptySpace(const gui::LinearLayoutElement& horizontal, bool use_min_radiuses=false);
	void adjustElementY(gui::Element& e, int row, const util::Coord& min_height);
	util::Coord calcSumOfLayoutVerticalMin() const;
	void limitComponents();
	void adjustLayouts();
	
	EditorView* view;

	bool active;

	gui::LinearLayoutElement verticalLayout;
	util::DynArray<gui::LinearLayoutElement> horizontalLayouts; // Inside verticalLayout
	util::DynArray<std::shared_ptr<BaseEditorComponentUi>> componentUis;
	util::DynArray<gui::ComboBoxElement> addCombos;
};

}}} // ui::game::editor
} // clover

#endif // CLOVER_UI_EDITOR_EDITORVIEW_UI_HPP