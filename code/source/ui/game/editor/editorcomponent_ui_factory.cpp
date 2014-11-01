#include "editorcomponent_ui_factory.hpp"
#include "game/editor/editorcomponent.hpp"
#include "editorcomponent_ui.hpp"

#define EC_HEADERS
#include "game/editor/components/components.def"
#undef EC_HEADERS

namespace clover {
namespace ui { namespace game { namespace editor {

std::shared_ptr<BaseEditorComponentUi> EditorComponentUiFactory::create(EditorComponent& comp){
	
#define EC(ui_type) \
	if (comp.getName() == ui_type::getComponentName()){ \
		ui_type::ComponentType* ptr= dynamic_cast<ui_type::ComponentType*>(&comp); \
		ensure(ptr); \
		return (std::shared_ptr<BaseEditorComponentUi>(new ui_type(*ptr))); \
	}
#include "game/editor/components/components.def"
#undef EC
	
	throw global::Exception("Corresponding EditorComponentUi not found for component %s", comp.getName().cStr());
}

}}} // ui::game::editor
} // clover