#ifndef CLOVER_UI_EDITOR_EDITORCOMPONENT_UI_FACTORY_HPP
#define CLOVER_UI_EDITOR_EDITORCOMPONENT_UI_FACTORY_HPP

#include "build.hpp"
#include "common.hpp"
#include "baseeditorcomponent_ui.hpp"

#include <memory>

namespace clover {
namespace game { namespace editor {

class EditorComponent;

}} // game::editor
namespace ui { namespace game { namespace editor {

class EditorComponentUiFactory {
public:
	static std::shared_ptr<BaseEditorComponentUi> create(EditorComponent&);
};

}}} // ui::game::editor
} // clover

#endif // CLOVER_UI_EDITOR_EDITORCOMPONENT_UI_FACTORY_HPP