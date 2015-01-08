#ifndef CLOVER_GAME_EDITOR_EXTENSION_MGR_HPP
#define CLOVER_GAME_EDITOR_EXTENSION_MGR_HPP

#include "build.hpp"
#include "editorextension.hpp"
#include "util/dyn_array.hpp"
#include "util/unique_ptr.hpp"

namespace clover {
namespace game { namespace editor {

/// Handles connections to other programs (like Blender)
class EditorExtensionMgr {
public:
	EditorExtensionMgr();

	void update();

private:
	using ExtensionPtr= util::UniquePtr<EditorExtension>;

	template <typename T>
	EditorExtension& createExtension(){
		extensions.pushBack(ExtensionPtr(new T));
		return *extensions.back();
	}

	util::DynArray<ExtensionPtr> extensions;
};

}} // game::editor
} // clover

#endif // CLOVER_GAME_EDITOR_EXTENSION_MGR_HPP
