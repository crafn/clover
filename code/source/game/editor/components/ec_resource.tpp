template <typename T>
ResourceEc<T>::ResourceEc()
	: resource(nullptr){
	
	listenForEvent(global::Event::OnEditorResourceSelect);
}

template <typename T>
void ResourceEc<T>::onEvent(global::Event& e){
	switch(e.getType()){
		case global::Event::OnEditorResourceSelect: {
			if (e(global::Event::TypeName).getStr() == resources::ResourceTraits<ResourceType>::typeName()){
				// Corresponding type of resource selected
				auto res= static_cast<ResourceType*>(e(global::Event::Resource).getPtr<resources::Resource>());
				ensure(res);
				selectResource(*res);
			}
		}
		break;
		
		default: break;
	}
}

template <typename T>
util::DynArray<resources::ResourceFilePath> ResourceEc<T>::getResourceFilePaths() const {
	return resources::gCache->getResourceFilePaths();
}

template <typename T>
auto ResourceEc<T>::createResource(const ResourceIdentifierValue& id, const resources::ResourceFilePath& group) const -> ResourceType& {
	return resources::gCache->createResource<ResourceType>(id, group);
}

template <typename T>
void ResourceEc<T>::selectResource(ResourceType& res){
	resource= &res;
	for (auto& m : resourceSelectCallbacks)
		m();
}

template <typename T>
void ResourceEc<T>::clearSelectedResource(){
	resource= nullptr;
	for (auto& m : resourceSelectCallbacks)
		m();
}
