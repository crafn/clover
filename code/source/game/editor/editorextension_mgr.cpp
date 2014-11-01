#include "editorextension_mgr.hpp"
#include "extensions/ee_blender.hpp"
#include "global/cfg_mgr.hpp"

namespace clover {
namespace game { namespace editor {

EditorExtensionMgr::EditorExtensionMgr(){
	if (global::gCfgMgr->get<bool>("dev::enableEditorExtensions", false))
		createExtension<BlenderEE>();
}

void EditorExtensionMgr::update(){
	for (auto& m : extensions){
		m->update();
	}
}

}} // game::editor
} // clover