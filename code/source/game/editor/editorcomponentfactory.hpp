#ifndef CLOVER_GAME_EDITOR_EDITORCOMPONENTFACTORY_HPP
#define CLOVER_GAME_EDITOR_EDITORCOMPONENTFACTORY_HPP

#include "build.hpp"
#include "editorcomponent.hpp"
#include "util/dyn_array.hpp"

#include <memory>

namespace clover {
namespace game { namespace editor {

class EditorComponentFactory {
public:

	/// Creates and returns a new EditorComponent by it's name (EditorComponentTraits<Component>::getName() == name)
	static std::shared_ptr<EditorComponent> create(const util::Str8& name);

	/// Creates a component of type Comp
	template <typename Comp>
	static std::shared_ptr<Comp> create(){
		return (std::shared_ptr<Comp>(new Comp()));
	}

	static util::DynArray<util::Str8> getComponentNames();

	/// @return Name of EditorComponent type which can handle asked resource
	static util::Str8 getCorrespondingResourceComponentName(const util::Str8& resource_type_name);
};

}} // game::editor
} // clover

#endif // CLOVER_GAME_EDITOR_EDITORCOMPONENTFACTORY_HPP