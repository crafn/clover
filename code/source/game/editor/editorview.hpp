#ifndef CLOVER_GAME_EDITOR_EDITORVIEW_HPP
#define CLOVER_GAME_EDITOR_EDITORVIEW_HPP

#include "build.hpp"
#include "editorcomponent.hpp"
#include "util/dyn_array.hpp"

#include <memory>

namespace clover {
namespace game { namespace editor {

/// A screenful of editor
class EditorView {
public:
	EditorView();
	EditorView(const EditorView&)= delete;
	EditorView(EditorView&&)= default;
	virtual ~EditorView();

	void setActive(bool b=true);

	EditorComponent& createComponent(const util::Str8& name);
	void destroyComponent(EditorComponent& comp);

	bool hasComponent(const util::Str8& type_name);

private:
	bool active;
	util::DynArray<std::shared_ptr<EditorComponent>> components;
};

}} // game::editor
} // clover

#endif // CLOVER_GAME_EDITOR_EDITORVIEW_HPP