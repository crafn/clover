template <typename T>
SubCache<T>::SubCache(){
	print(debug::Ch::Resources, debug::Vb::Trivial, "(%s) SubCache created", getResourceTypeName().cStr());
	errorResource.createErrorResource();
}

template <typename T>
SubCache<T>::~SubCache(){
	print(debug::Ch::Resources, debug::Vb::Trivial, "(%s) Destroying SubCache", getResourceTypeName().cStr());
}

template <typename T>
const Resource& SubCache<T>::getResource(const util::ObjectNode& id){
	return getResource(id.getValue<IdentifierValue>());
}

template <typename T>
const T& SubCache<T>::getResource(const IdentifierValue& id){
	auto res= findResource(id);
	if (res == nullptr){
		print(debug::Ch::Resources, debug::Vb::Moderate, "(%s) Resource not found: %s",
			getResourceTypeName().cStr(), ResourceTraits<T>::stringFromIdentifier(id).cStr());
		return createMissingResource(id);
	}
	
	if (res->isResourceObsolete()){
		print(debug::Ch::Resources, debug::Vb::Moderate, "(%s) Using obsolete resource: %s",
			getResourceTypeName().cStr(), ResourceTraits<T>::stringFromIdentifier(id).cStr());
	}

	return *res;
}

template <typename T>
T& SubCache<T>::createResource(const IdentifierValue& id, const ResourceFilePath& path){
	T* res_ptr= nullptr;
	
	auto res_it= resources.find(id);
	if (res_it != resources.end())
		res_ptr= res_it->second.get();
	
	if (res_ptr != nullptr && !res_ptr->isResourceObsolete())
		throw resources::ResourceException("(%s) Resource already exists: %s",
			getResourceTypeName().cStr(),
			resources::ResourceTraits<ResourceType>::stringFromIdentifier(id).cStr());
	
	if (res_ptr){
		ensure(res_ptr->isResourceObsolete());
		
		print(debug::Ch::Resources, debug::Vb::Trivial, "(%s) Reviving obsolete resource: %s",
			getResourceTypeName().cStr(),
			ResourceTraits<ResourceType>::stringFromIdentifier(id).cStr());
			
		res_ptr->setResourceObsolete(false);
		
		auto& reslist= resourcesByFile[res_ptr->getResourceFilePath()];
		auto it= reslist.find(id);
		if (it != reslist.end())
			reslist.erase(it);

	}
	else {
		res_ptr= &createNewResource(id);
	}	
	
	ensure(res_ptr);
	
	resourcesByFile[path][id]= res_ptr;
	res_ptr->setResourceFile(path);
	
	return *res_ptr;
	
}

template <typename T>
const T* SubCache<T>::findResource(const IdentifierValue& id){
	return findResourceImpl(id);
}

template <typename T>
void SubCache<T>::parseResource(const SerializedResource& serialized){
	const util::ObjectNode& attrib_object= serialized.getResourceAttributes();
	const auto& path= serialized.getPath();

	if (!attrib_object.isObject())
		throw ResourceException("Not an object");
	
	util::ObjectNode::MemberNames field_names= attrib_object.getMemberNames();

	auto key_iter= std::find(field_names.begin(), field_names.end(), ResourceTraits<ResourceType>::identifierKey().cStr());

	if (key_iter == field_names.end())
		throw ResourceException("Identifier field not found");

	uint32 identifier_i= key_iter - field_names.begin();
	util::ObjectNode identifier_value= attrib_object.get(field_names[identifier_i]);
	
	typedef typename ResourceTraits<ResourceType>::IdentifierAttributeType AttributeType;
	AttributeInitializer<AttributeType> init;
	
	// Create attribute for parsing the identifier-value
	Attribute<AttributeType> identifier_attribute(init);
		
	identifier_attribute.set(AttributeParseInfo(identifier_value, path.directoryFromRoot()));
	
	// For debug messages
	util::Str8 resource_key_str= ResourceTraits<ResourceType>::stringFromIdentifier(identifier_attribute.get());

	try {
	
		T* res_ptr= 0;
		
		auto it= resources.find(identifier_attribute.get());
		if (it == resources.end()){
			// New resource
			res_ptr= &createNewResource(identifier_attribute.get());
		}
		else {
			// Existing resource
			res_ptr= it->second.get();
		}
		
		ensure(res_ptr);
		res_ptr->setResourceObsolete(false);
		
		ensure_msg(resourcesByFile.find(path) != resourcesByFile.end(), "Invalid path: %s", path.fromRoot().cStr());
		
		resourcesByFile[path][identifier_attribute.get()]= res_ptr;
		res_ptr->setResourceFile(path);
		

		// Parse and set attributes except identifier, which has been set in resource creation
		for (uint32 attr_i=0; attr_i<field_names.size(); ++attr_i){
			
			if (attr_i == identifier_i) continue; // Don't parse identifier-field again
			util::Str8 key= field_names[attr_i];

			try {
				util::ObjectNode attr_field= attrib_object.get(field_names[attr_i]);
				res_ptr->getResourceAttribute(key).set(AttributeParseInfo(attr_field, path.directoryFromRoot()));
			}
			catch (const ResourceException& e){
				print(debug::Ch::Resources, debug::Vb::Critical,
					"(%s) Attribute %s parsing failed for %s", getResourceTypeName().cStr(), key.cStr(), resource_key_str.cStr());
			}
		}
		
		print(debug::Ch::Resources, debug::Vb::Trivial,
			"(%s) %s parsed", getResourceTypeName().cStr(), resource_key_str.cStr());
	}
	catch (const ResourceException& e){
		print(debug::Ch::Resources, debug::Vb::Critical,
			"(%s) Parsing failed: %s", getResourceTypeName().cStr(), resource_key_str.cStr());
	}
}

template <typename T>
void SubCache<T>::parseResourceSection(const util::ObjectNode& root, const ResourceFilePath& path){
	if (!root.isArray())
			throw ResourceException("Root is not an array");
			
	// util::Set all resources from that resource file obsolete
	// Found resources will be set unobsolete so resources removed by hand will be obsolete after parsing the file
	for (auto& res_pair : resourcesByFile[path]){
		res_pair.second->setResourceObsolete();
	}
		
	for (uint32 res_i=0; res_i<root.size(); ++res_i){
		util::ObjectNode attrib_object= root.get(res_i);
		parseResource(SerializedResource(path, getResourceTypeName(), attrib_object));
	}
	
}

template <typename T>
util::ObjectNode SubCache<T>::generateSection(const ResourceFilePath& path){
	util::ObjectNode root;
	
	ensure(!path.relative().empty());
	auto& resource_map= resourcesByFile[path];
	
	for (auto& res_pair : resource_map){
		const auto& res= res_pair.second;
		if (res->isResourceObsolete()) continue;

		util::ObjectNode object_value= std::move(res->getSerializedResource().getResourceAttributes());
		root.append(object_value);

	}
	
	return (root);
}

template <typename T>
void SubCache<T>::preLoad(){
	for (auto& m : resources){
		if (m.second->getResourceState() == Resource::State::Uninit){
			try {
				m.second->resourceUpdate(false);
			}
			catch(ResourceException e){
				print(debug::Ch::Resources, debug::Vb::Critical,
					"(%s) resourceUpdate(..) failed for %s: %s",
					getResourceTypeName().cStr(),
					ResourceTraits<ResourceType>::stringFromIdentifier(m.first).cStr(), e.what());
			}
		}
	}
}

template <typename T>
void SubCache<T>::update(){
	for (auto& m : resources){
		T& res= *m.second;
		
		if (res.getResourceState() == Resource::State::Uninit){
			// Initialization
			res.resourceUpdate(false);
			continue;
		}
		
		res.updateStaleness();
		
		if (ResourceTraits<T>::expired(res) && res.getResourceState() == Resource::State::Loaded){
			// Too old, unload
			res.resourceUpdate(false);

			print(debug::Ch::Resources, debug::Vb::Trivial,
				"(%s) Resource expired: %s",
				getResourceTypeName().cStr(),
				ResourceTraits<ResourceType>::stringFromIdentifier(m.first).cStr());

		}
		else if (!ResourceTraits<T>::expired(res) && res.getResourceState() == Resource::State::Unloaded){
			// Is used, load
			print(debug::Ch::Resources, debug::Vb::Trivial,
				"(%s) Loading resource: %s",
				getResourceTypeName().cStr(),
				ResourceTraits<ResourceType>::stringFromIdentifier(m.first).cStr());
			res.resourceUpdate(true);
		}
		else if (res.getResourceState() == Resource::State::Loading){
			// Update resource loading
			res.resourceUpdate(true);
		}
		else if (res.getResourceState() == Resource::State::Unloading){
			// Update resource unloading
			res.resourceUpdate(false);
		}
	}
}

template <typename T>
const util::DynArray<Resource*>& SubCache<T>::getGenericResources() const {
	/// @todo Don't generate this every time
	static util::DynArray<Resource*> ret;
	ret.clear();
	
	for (auto& m : resources){
		ret.pushBack(m.second.get());
	}
	
	return ret;
}

template <typename T>
util::DynArray<T*> SubCache<T>::getResources() const {
	util::DynArray<T*> ret;

	for (auto& m : resources){
		ret.pushBack(m.second.get());
	}
	
	return (ret);
}

template <typename T>
void SubCache<T>::setResourceToErrorState(const IdentifierValue& id){
	T* res= findResourceImpl(id);
	if (res != nullptr){
		res->createErrorResource();
	}
}

template <typename T>
T& SubCache<T>::createNewResource(const IdentifierValue& id){
	ensure(resources.count(id) == 0);
	
	T* ptr= new T();
	resources[id]= util::UniquePtr<T>(ptr);
	ptr->getResourceAttribute(ResourceTraits<T>::identifierKey()).template set<IdentifierAttributeType>(id); // util::Set name
	return *ptr;
}

template <typename T>
T& SubCache<T>::createMissingResource(const IdentifierValue& id){
	T& ret= createNewResource(id);
	ret.setResourceObsolete(); // Don't save to file
	ret.createErrorResource(); // In error state
	
	print(debug::Ch::Resources, debug::Vb::Moderate, "(%s) Missing resource created: %s",
		getResourceTypeName().cStr(),
		ResourceTraits<ResourceType>::stringFromIdentifier(id).cStr());
	return ret;
}

template <typename T>
T* SubCache<T>::findResourceImpl(const IdentifierValue& id){
	auto it= resources.find(id);
	if (it == resources.end()){
		return nullptr;
	}
	return it->second.get();
}