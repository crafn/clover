#ifndef CLOVER_RESOURCES_CACHE_HPP
#define CLOVER_RESOURCES_CACHE_HPP

#include "animation/bvh_util.hpp"
#include "build.hpp"
#include "util/dyn_array.hpp"
#include "util/hashmap.hpp"
#include "util/map.hpp"
#include "util/math.hpp"
#include "util/time.hpp"
#include "util/string.hpp"
#include "resources/resource.hpp"
#include "resources/basesubcache.hpp"
#include "resources/subcache.hpp"

#define DEFAULT_RES_PATH "../../resources/gamedata/"
#define DEFAULT_RES_PATH2 "./data/"

#include <string>
#include <iostream>

namespace clover {
namespace visual {

class Font;

} // visual
namespace resources {

/// Owner of resources
class ENGINE_API Cache {
public:
	static const util::Str8 shaderPath;

	Cache();
	Cache(const Cache&)= delete;
	Cache(Cache&&)= delete;
	virtual ~Cache();

	Cache& operator=(const Cache&)= delete;
	Cache& operator=(Cache&&)= delete;

	/// Call once per frame
	void update();

	void writeAllResources();

	const Resource& getResource(const ResourceId& res_id);

	template <typename T>
	const T& getResource(const typename ResourceTraits<T>::IdentifierValue& identifier){
		return getSubCache<T>().getResource(identifier);
	}

	template <typename T>
	const T* findResource(const typename ResourceTraits<T>::IdentifierValue& identifier){
		return getSubCache<T>().findResource(identifier);
	}

	template <typename T>
	const T& getErrorResource(){
		return getSubCache<T>().getErrorResource();
	}

	template <typename T>
	void setResourceToErrorState(const typename ResourceTraits<T>::IdentifierValue& identifier){
		getSubCache<T>().setResourceToErrorState(identifier);
	}

	const util::DynArray<Resource*>& getResources() const;

	template <typename T>
	T& createResource(const typename ResourceTraits<T>::IdentifierValue& id, const resources::ResourceFilePath& group){
		return getSubCache<T>().createResource(id, group);
	}

	void parseResource(const SerializedResource& serialized);

	visual::Font& getFont(const util::Str8& s);

	util::Str8 getResourceRootPath() const { return resourcePath; }

	const util::DynArray<ResourceFilePath>& getResourceFilePaths() const { return resourceFilePaths; }

	template <typename T>
	typename ResourceTraits<T>::SubCacheType& getSubCache(){
		auto it= subCaches.find(ResourceTraits<T>::typeName());
		ensure_msg(it != subCaches.end(), "SubCache not found");
		typename ResourceTraits<T>::SubCacheType& sub_cache= static_cast<typename ResourceTraits<T>::SubCacheType&>(*it->second);
		return sub_cache;
	}

	/// Create all resources of type `T` defined in resource files
	template <typename T>
	void preLoad();

	/// Unload all resources of type `T`, and destroy related subcache
	template <typename T>
	void unload();

private:
	template <typename T>
	void createSubCache();
	void parseResourceFile(
			const ResourceFilePath& path,
			const util::Str8& only_this= "");
	void processResData(
			util::ObjectNode& root,
			const ResourceFilePath& path_to_resource_file,
			const util::Str8& only_this= "");

	util::HashMap<util::Str8, std::unique_ptr<BaseSubCache>> subCaches;
	util::DynArray<ResourceFilePath> resourceFilePaths;
	util::LinkedList<std::shared_ptr<util::FileWatcher>> resourceFileWatchers;

	// Resource root
	util::Str8 resourcePath;

	/// @todo Replace with resource system
	void loadFonts();
};

} // resources
namespace util {

template <>
struct TypeStringTraits<resources::Cache> {
	static util::Str8 type(){ return "resources::Cache"; }
};

} // util
} // clover

#endif // CLOVER_RESOURCES_CACHE_HPP
