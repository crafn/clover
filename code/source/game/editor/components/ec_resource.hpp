#ifndef CLOVER_GAME_EDITOR_EC_RESOURCE_HPP
#define CLOVER_GAME_EDITOR_EC_RESOURCE_HPP

#include "../editorcomponent.hpp"
#include "build.hpp"
#include "global/env.hpp"
#include "resources/cache.hpp"
#include "resources/resource.hpp"

namespace clover {
namespace game { namespace editor {

/// Some useful tools for resource editor components
class BaseResourceEc : public EditorComponent {
public:
	virtual ~BaseResourceEc(){}
};

/// Component for editing a certain type of resources in editor
template <typename T>
class ResourceEc : public BaseResourceEc {
public:
	DECLARE_EDITORCOMPONENT(ResourceEc<T>);

	using ResourceType= T;
	using ResourceIdentifierValue= typename resources::ResourceTraits<ResourceType>::IdentifierValue;
	
	ResourceEc();
	virtual ~ResourceEc(){}
	
	virtual void onEvent(global::Event& e);
	
	util::DynArray<resources::ResourceFilePath> getResourceFilePaths() const;
	ResourceType& createResource(const ResourceIdentifierValue& id, const resources::ResourceFilePath& group) const;
	void selectResource(ResourceType& res);
	
	ResourceType* getSelectedResource() const { return resource; }
	
	// Called also when resource is unselected
	using ResourceSelectCallback= std::function<void ()>;
	void addOnResourceSelectCallback(ResourceSelectCallback cb){
		resourceSelectCallbacks.pushBack(cb);
	}
	
	void clearSelectedResource();
	
private:
	ResourceType* resource;
	util::DynArray<ResourceSelectCallback> resourceSelectCallbacks;
};


template <>
template <typename T>
struct EditorComponentTraits<ResourceEc<T>> {
	static const util::Str8 name(){ return resources::ResourceTraits<T>::typeName(); }
};

#include "ec_resource.tpp"

}} // game::editor
} // clover

#endif // CLOVER_GAME_EDITOR_EC_RESOURCE_HPP
