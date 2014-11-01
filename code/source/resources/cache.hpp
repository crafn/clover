#ifndef CLOVER_RESOURCES_CACHE_HPP
#define CLOVER_RESOURCES_CACHE_HPP

#include "animation/bvh_util.hpp"
#include "build.hpp"
#include "script/reference.hpp"
#include "util/dyn_array.hpp"
#include "util/hashmap.hpp"
#include "util/map.hpp"
#include "util/math.hpp"
#include "util/profiling.hpp"
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
class Shader;
class Model;
class Texture;
class TriMesh;
class Material;

} // visual
namespace resources {

/// Hard coded shaders
enum ShaderId {
	Shader_Generic,
	Shader_Blur,
	Shader_ShadowCaster,
	Shader_ShadowMap,
	Shader_Particle,
	Shader_Last
};

/// Owner of resources
class Cache : public script::NoCountReference {
public:
	static const util::Str8 shaderPath;

	Cache();
	Cache(const Cache&)= delete;
	Cache(Cache&&)= delete;
	virtual ~Cache();

	Cache& operator=(const Cache&)= delete;
	Cache& operator=(Cache&&)= delete;

	/// Parses resources and loads essential stuff
	void preLoad();

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

	//
	// Shaders
	//

	/// @todo Remove
	util::DynArray<visual::Shader>& getShaders(ShaderId id);

	struct GenericShaderType {
		uint32 lightCount;
		bool normalMap;
		bool colorMap;
		bool envShadowMap;
		bool curve;
		bool sway;

		GenericShaderType(): lightCount(0), normalMap(false), colorMap(false), envShadowMap(false), curve(false), sway(false){}
		bool operator==(const GenericShaderType& t) const {
			return 	lightCount == t.lightCount &&
					normalMap == t.normalMap &&
					colorMap == t.colorMap &&
					envShadowMap == t.envShadowMap &&
					curve == t.curve &&
					sway == t.sway;
		}

		uint32 getId() const { return lightCount*32 + (int)normalMap + (int)colorMap*2 + (int) envShadowMap*4 + (int)curve*8 + (int)sway*16; }

		bool operator<(const GenericShaderType& t) const {
			return getId() < t.getId();
		}

	};

	visual::Shader& getGenericShader(GenericShaderType t);
	uint32 getShaderCount();

	visual::Font& getFont(const util::Str8& s);

	util::Str8 getResourceRootPath() const { return resourcePath; }

	const util::DynArray<ResourceFilePath>& getResourceFilePaths() const { return resourceFilePaths; }

	template <typename T>
	typename ResourceTraits<T>::SubCacheType& getSubCache(){
		PROFILE_("resources");
		auto it= subCaches.find(ResourceTraits<T>::typeName());
		ensure_msg(it != subCaches.end(), "SubCache not found");
		typename ResourceTraits<T>::SubCacheType& sub_cache= static_cast<typename ResourceTraits<T>::SubCacheType&>(*it->second);
		return sub_cache;
	}

private:
	template <typename T>
	void createSubCache();

	util::HashMap<util::Str8, std::unique_ptr<BaseSubCache>> subCaches;

	util::DynArray<ResourceFilePath> resourceFilePaths;
	util::LinkedList<std::shared_ptr<util::FileWatcher>> resourceFileWatchers;
	void parseResourceFile(const ResourceFilePath& path);

	void processJson(util::ObjectNode& root, const ResourceFilePath& path_to_resource_file);

	// Resource root
	util::Str8 resourcePath;

	// Shaders

	util::Map<GenericShaderType, visual::Shader> genericShaderMap;
	util::Str8 	genericShaderFragSrc,
					genericShaderVertSrc,
					genericShaderGeomSrc;

	util::DynArray<visual::Shader> shadowCasterShaders;
	util::DynArray<visual::Shader> shadowMapShaders;
	util::DynArray<visual::Shader> particleShaders;

	/// @todo Replace with resource system
	void loadFonts();
	void loadShaders();
	void createGenericShader(GenericShaderType t);
};

extern Cache* gCache;

} // resources
namespace util {

template <>
struct TypeStringTraits<resources::Cache> {
	static util::Str8 type(){ return "resources::Cache"; }
};

} // util
} // clover

#endif // CLOVER_RESOURCES_CACHE_HPP