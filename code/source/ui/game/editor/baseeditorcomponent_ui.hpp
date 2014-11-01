#ifndef CLOVER_UI_EDITOR_BASEEDITORCOMPONENTUI_HPP
#define CLOVER_UI_EDITOR_BASEEDITORCOMPONENTUI_HPP

#include "build.hpp"
#include "global/eventreceiver.hpp"
#include "util/coord.hpp"

namespace clover {
namespace gui {

class Element;

} // gui
namespace game { namespace editor {

class EditorComponent;

}} // game::editor
namespace ui { namespace game { namespace editor {
using namespace clover::game::editor;

class BaseEditorComponentUi : public global::EventReceiver {
public:
	BaseEditorComponentUi(){}
	BaseEditorComponentUi(const BaseEditorComponentUi&)= delete;
	BaseEditorComponentUi(BaseEditorComponentUi&&)= delete;
	BaseEditorComponentUi& operator=(const BaseEditorComponentUi&)= delete;
	BaseEditorComponentUi& operator=(BaseEditorComponentUi&&)= delete;
	virtual ~BaseEditorComponentUi(){}
	
	virtual void onEvent(global::Event&){}
	
	// Returned element is added to editorview's gui
	virtual gui::Element& getSuperGuiElement()= 0;
	virtual EditorComponent& getComponent()= 0;
	
	virtual void setMaxRadius(const util::Coord&)= 0;
	virtual const util::Coord& getMaxRadius() const = 0;
	
	virtual void update(){}
	
protected:
};

}}} // ui::game::editor
} // clover

#endif // CLOVER_UI_EDITOR_BASEEDITORCOMPONENTUI_HPP