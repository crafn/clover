#include "editorcomponent_ui.hpp"

#define EC_HEADERS
#include "game/editor/components/components.def"
#undef EC_HEADERS

namespace clover {
namespace ui { namespace game { namespace editor {

util::Coord RuntimeEditorComponentUiTraits::defaultRadius(const util::Str8& comp_type){
	#define EC(ui_type) \
		if (EditorComponentTraits<ui_type::ComponentType>::name() == comp_type) \
			return EditorComponentUiTraits<ui_type::ComponentType>::defaultRadius();
	#include "game/editor/components/components.def"
	#undef EC
	
	throw global::Exception("Invalid EditorComponent type: %s", comp_type.cStr());
}

}}} // ui::game::editor
} // clover