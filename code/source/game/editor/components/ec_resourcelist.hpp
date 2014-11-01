#ifndef CLOVER_GAME_EDITOR_EC_RESOURCELIST_HPP
#define CLOVER_GAME_EDITOR_EC_RESOURCELIST_HPP

#include "../editorcomponent.hpp"
#include "build.hpp"

namespace clover {
namespace resources {

class Resource;

} // resources
namespace game { namespace editor {

class ResourceListEc;

template <>
struct EditorComponentTraits<ResourceListEc> {
	static const util::Str8 name(){ return "ResourceList"; }
};

class ResourceListEc : public EditorComponent {
public:
	DECLARE_EDITORCOMPONENT(ResourceListEc);
	
	virtual ~ResourceListEc(){}
	
	/// Performs a search for resources
	/// @param search_str 	Contains the keywords which are looked for in resource names
	///						There's an or -relation between words
	/// @param use_tags	If true, search also name tag of the resource
	util::DynArray<resources::Resource*> search(const util::Str8& search_str, uint32 max_results, bool use_tags= true);
};

}} // game::editor
} // clover

#endif // CLOVER_GAME_EDITOR_EC_RESOURCELIST_HPP