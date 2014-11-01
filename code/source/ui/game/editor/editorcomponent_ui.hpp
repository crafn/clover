#ifndef CLOVER_UI_EDITOR_EDITORCOMPONENT_UI_HPP
#define CLOVER_UI_EDITOR_EDITORCOMPONENT_UI_HPP

#include "build.hpp"
#include "common.hpp"
#include "gui/element_button.hpp"
#include "gui/element_panel.hpp"
#include "gui/element_textlabel.hpp"
#include "gui/element_layout_linear.hpp"
#include "baseeditorcomponent_ui.hpp"
#include "game/editor/editorcomponent.hpp"
#include "global/event.hpp"
// .tpp
#include "ui/userinput.hpp"

namespace clover {
namespace ui { namespace game { namespace editor {

template <typename T> // EditorComponent T
class EditorComponentUi : public BaseEditorComponentUi {
public:
	typedef T ComponentType;
	typedef EditorComponentUi<T> This;
	
	EditorComponentUi(T& comp);
	virtual ~EditorComponentUi();
	
	ComponentType& getComponent();
	static util::Str8 getComponentName();
	gui::Element& getSuperGuiElement();
	
	
	gui::PanelElement& getContentElement(){ return contentPanel; }
	void setAdditionalTitle(const util::Str8& additional_str);
	
	virtual void setMaxRadius(const util::Coord& max_radius);
	virtual const util::Coord& getMaxRadius() const { return maxRadius; }
	
private:
	util::Coord radius;
	util::Coord maxRadius;
	
protected:
	T* component;

	void setRadius(const util::Coord& rad);
	const util::Coord& getRadius() const;
	
	virtual void onResize(); // Called when radius is changed
	
private:
	static constexpr real64 resizeButtonVerticalRad= 0.018; //VF

	gui::PanelElement bgPanel;
	gui::LinearLayoutElement panelLayout;
	gui::TextLabelElement titleLabel;
	
	gui::LinearLayoutElement resizeButtonLayout;
	gui::ButtonElement resizeButton;
	bool resizing;
	util::Coord resizeRadius;
	
protected:
	gui::PanelElement contentPanel;
};

template <typename T> // EditorComponent T
struct EditorComponentUiTraits {
	static util::Coord defaultRadius(){ return util::Coord::VF(0.4); }
};

struct RuntimeEditorComponentUiTraits {
	static util::Coord defaultRadius(const util::Str8& comp_type);
};

#include "editorcomponent_ui.tpp"

}}} // ui::game::editor
} // clover

#endif // CLOVER_UI_EDITOR_EDITORCOMPONENT_UI_HPP