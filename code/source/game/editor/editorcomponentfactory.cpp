#include "editorcomponentfactory.hpp"

#define EC_HEADERS
#include "components/components.def"
#undef EC_HEADERS

#include <type_traits>

namespace clover {
namespace game { namespace editor {

std::shared_ptr<EditorComponent> EditorComponentFactory::create(const util::Str8& name){

#define EC(ui_type) if (EditorComponentTraits<ui_type::ComponentType>::name() == name){ \
		auto ptr= std::shared_ptr<EditorComponent>(new ui_type::ComponentType()); \
		return ptr; \
	}
#include "components/components.def"
#undef EC

	throw global::Exception("Corresponding EditorComponent not found for name %s", name.cStr());
}

util::DynArray<util::Str8> EditorComponentFactory::getComponentNames(){
	
	util::DynArray<util::Str8> ret;
	
#define EC(ui_type) ret.pushBack(EditorComponentTraits<ui_type::ComponentType>::name());
#include "components/components.def"
#undef EC
	
	return (ret);
}

/// @param T EditorComponent
template <typename T>
typename std::enable_if<std::is_base_of<BaseResourceEc, T>::value, bool>::type canHandleResource(const util::Str8& resource_type_name){
	// Only ResourceEcs have ::ResourceType so enable_if is needed
	if (resources::ResourceTraits<typename T::ResourceType>::typeName() == resource_type_name)
		return true;
	return false;
}

template <typename T>
typename std::enable_if<!std::is_base_of<BaseResourceEc, T>::value, bool>::type canHandleResource(const util::Str8& resource_type_name){
	return false;
}

util::Str8 EditorComponentFactory::getCorrespondingResourceComponentName(const util::Str8& resource_type_name){
	#define EC(ui_type) \
		if (canHandleResource<ui_type::ComponentType>(resource_type_name)) \
			return EditorComponentTraits<ui_type::ComponentType>::name();
	#include "components/components.def"
	#undef EC
	
	throw global::Exception("No component found for resource type %s", resource_type_name.cStr());
}

}} // game::editor
} // clover