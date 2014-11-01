#ifndef CLOVER_GAMELOGIC_EDITOR_EDITOR_HPP
#define CLOVER_GAMELOGIC_EDITOR_EDITOR_HPP

#include "build.hpp"
#include "editorview.hpp"
#include "editorextension_mgr.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace game { namespace editor {

/// Editor logic
class Editor {
public:
	static constexpr int32 viewCount= 4;

	Editor();
	virtual ~Editor();

	void update();

private:
	util::DynArray<EditorView> views;
	EditorExtensionMgr extensionMgr;
};

}} // game::editor
} // clover

#endif // CLOVER_GAMELOGIC_EDITOR_EDITOR_HPP