#ifndef CLOVER_RESOURCES_SUBCACHE_HPP
#define CLOVER_RESOURCES_SUBCACHE_HPP

#include "build.hpp"
#include "basesubcache.hpp"
#include "pathattributevalue.hpp"
#include "resources/resource.hpp"
#include "resources/serializedresource.hpp"
#include "util/dyn_array.hpp"
#include "util/objectnode.hpp"
// .tpp
#include "debug/debugprint.hpp"
#include "resources/exception.hpp"
#include "resources/attributeparseinfo.hpp"
#include "util/string.hpp"

#include <memory>

namespace clover {
namespace resources {

template <typename T>
class SubCache : public BaseSubCache {
	typedef typename ResourceTraits<T>::IdentifierValue IdentifierValue;
	typedef typename ResourceTraits<T>::IdentifierAttributeType IdentifierAttributeType;
public:
	typedef T ResourceType;

	SubCache();
	SubCache(SubCache&&)= default;
	SubCache(const SubCache&)= delete;
	virtual ~SubCache();

	const Resource& getResource(const util::ObjectNode& id);
	const T& getResource(const IdentifierValue& id);
	T& createResource(const IdentifierValue& id, const ResourceFilePath& path);
	virtual const T* findResource(const IdentifierValue& id);
	virtual const T& getErrorResource(){ return errorResource; }

	static util::Str8 resourceTypeName(){ return (ResourceTraits<T>::typeName()); }
	virtual util::Str8 getResourceTypeName() const { return resourceTypeName(); }

	virtual void parseResource(const SerializedResource& res);
	virtual void parseResourceSection(const util::ObjectNode& root, const ResourceFilePath& resource_file_path);
	virtual util::ObjectNode generateSection(const ResourceFilePath&);
	virtual void preLoad();
	virtual void update();
	virtual const util::DynArray<Resource*>& getGenericResources() const;
	util::DynArray<T*> getResources() const;

	void setResourceToErrorState(const IdentifierValue& identifier);

protected:

	virtual T& createNewResource(const IdentifierValue& identifier);

	T errorResource;

private:

	T& createMissingResource(const IdentifierValue& identifier);

	T* findResourceImpl(const IdentifierValue& id);

	util::HashMap<IdentifierValue, std::unique_ptr<T>> resources;

	util::HashMap<ResourceFilePath, util::HashMap<IdentifierValue, T*>> resourcesByFile;
};

#include "subcache.tpp"

} // resources
} // clover

#endif // CLOVER_RESOURCES_SUBCACHE_HPP