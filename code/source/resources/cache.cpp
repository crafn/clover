#include "cache.hpp"
#include "animation/bvh_util.hpp"
#include "hardware/device.hpp"
#include "visual/font_mgr.hpp"
#include "global/file.hpp"
// For shaders
#include "visual/particletype.hpp"
#include "visual/shader.hpp"
#include "physics/fluidparticle.hpp"

#define RESOURCE_HEADERS
#include "resourcetypes.def"
#undef RESOURCE_HEADERS

#include <boost/filesystem.hpp>
#include <fstream>
#include <json/json.h>
#include <json/writer.h>

#if OS == OS_WINDOWS
#include <io.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

namespace fs = boost::filesystem;

namespace clover {
namespace resources {

Cache* gCache;

Cache::Cache()
	: resourcePath(DEFAULT_RES_PATH){
	PROFILE_("resources");

	/// @todo Encapsulate
	bool exists= false;

	if ( access( DEFAULT_RES_PATH2, 0 ) == 0 ){
		struct stat status;
		stat( DEFAULT_RES_PATH2, &status );

		/// @todo Fix directory checking, didn't work on windows
		exists= true;

	}
	if (exists) resourcePath= DEFAULT_RES_PATH2;

	print(debug::Ch::Resources, debug::Vb::Trivial, "Resource path: %s", resourcePath.cStr());

}

Cache::~Cache(){
	print(debug::Ch::Resources, debug::Vb::Trivial, "Freeing resources");

	delete visual::gFontMgr;
	visual::gFontMgr=0;
}

void Cache::preLoad(){
	PROFILE_("resources");

#define RESOURCE(type_name) createSubCache<type_name>();
#include "resourcetypes.def"
#undef RESOURCE


	// Search all directories for .res files

	try {
		for (fs::recursive_directory_iterator end, dir(resourcePath.cStr()); dir != end; ++dir){


			util::Str8 path= dir->path().parent_path().string();
			if (path.length() <= resourcePath.length()){
				path= "./";
			}
			else {
				path.erase(0, resourcePath.length()); // util::Set the path to be relative from resource root
				path = "./" + path + "/";
			}
			util::Str8 filename= dir->path().filename().string();
			util::Str8 ext= dir->path().filename().extension().string();

			//print(debug::Ch::Resources, debug::Vb::Trivial, "File found: %s, %s, extension: %s", path.cStr(), filename.cStr(), ext.cStr());

			if (ext == ".res"){
					
				// New resource file
				resourceFilePaths.pushBack(ResourceFilePath(path, filename));
				
				// Watch the file
				resourceFileWatchers.pushBack(std::shared_ptr<util::FileWatcher>(new util::FileWatcher));
				ResourceFilePath p= resourceFilePaths.back();
				resourceFileWatchers.back()->setOnChangeCallback([=] (util::FileWatcher::FileEvent e) {
					parseResourceFile(p);
				});
				resourceFileWatchers.back()->setPath(resourceFilePaths.back().whole());
				resourceFileWatchers.back()->startWatching();
				
				
				parseResourceFile(resourceFilePaths.back());
			}
		}
	}
	catch (const fs::filesystem_error& e){
		print(debug::Ch::Resources, debug::Vb::Critical, "Filesystem error: %s", e.what());
	}

	#define RESOURCE(T) subCaches[ResourceTraits<T>::typeName()]->preLoad();
	#include "resourcetypes.def"
	#undef RSOURCE

	visual::gFontMgr= new visual::FontMgr();
	print(debug::Ch::Resources, debug::Vb::Trivial, "Loading fonts");
	loadFonts();
	print(debug::Ch::Resources, debug::Vb::Trivial, "Loading shaders");
	loadShaders();
}


void Cache::update(){
	PROFILE_("resources");
	for (auto& m : subCaches){
		m.second->update();
	}
}

const Resource& Cache::getResource(const ResourceId& res_id){
	ensure(subCaches.find(res_id.getTypeName()) != subCaches.end());
	
	return subCaches[res_id.getTypeName()]->getResource(res_id.getIdentifier());
}

void Cache::writeAllResources(){
	print(debug::Ch::Resources, debug::Vb::Trivial, "Stopping resource FileWatchers");
	for (auto& m : resourceFileWatchers)
		m->stopWatching();

	print(debug::Ch::Resources, debug::Vb::Trivial, "Writing all resources");
	hardware::gDevice->sleep(0.1); // Safety

	for (auto& path : getResourceFilePaths()){
		util::ObjectNode root(util::ObjectNode::Value::Object);
		
		for (auto& m : subCaches){
			util::ObjectNode section= m.second->generateSection(path);
			
			if (!section.empty())
				root[m.second->getResourceTypeName().cStr()]= section;
		}
		
		std::ofstream file((getResourceRootPath() + path.fromRoot()).cStr(), std::ios::binary);
		ensure(file);
		file << root.generateText().cStr();
		
	}
	
	for (auto& m : getResources()){
		m->onResourceWrite();
	}

	print(debug::Ch::Resources, debug::Vb::Trivial, "Writing finished");
	hardware::gDevice->sleep(0.1); // Safety

	for (auto& m : resourceFileWatchers)
		m->startWatching();
	
	print(debug::Ch::Resources, debug::Vb::Trivial, "Resource FileWatchers restored");
}

void Cache::loadFonts(){
	util::Str8 fontconf= global::File::readText("fonts.json");

	util::ObjectNode root;
	
	try {
		root.parseText(fontconf);
	}
	catch (resources::ResourceException&){
		release_ensure_msg(0, "ResourceMgr::loadFonts(): Parsing failed");
	}

	util::ObjectNode font_array_v= root.get("Fonts");
	release_ensure(font_array_v.isArray());

	for (uint32 i=0; i<font_array_v.size(); ++i){
		util::ObjectNode name_v= font_array_v[i].get("Name");
		util::ObjectNode files_v= font_array_v[i].get("Files");
		util::ObjectNode regular_v= files_v.get("Regular");
		visual::gFontMgr->createFont(resourcePath + util::Str8("fonts/") + regular_v.getValue<util::Str8>(), name_v.getValue<util::Str8>());
	}
}

void Cache::loadShaders(){
	#define SHADER_LOAD_BEGIN() try {
	#define SHADER_LOAD_END(name) \
		} catch(ResourceException& e){ print(debug::Ch::Resources, debug::Vb::Critical, "Error loading " name " shader"); throw e; }
	

	util::Str8 frag_src, vert_src, geom_src;

	frag_src= global::File::readText("shaders/generic.frag");
	vert_src= global::File::readText("shaders/generic.vert");
	geom_src= global::File::readText("shaders/generic.geom");

	genericShaderFragSrc= frag_src;
	genericShaderVertSrc= vert_src;
	genericShaderGeomSrc= geom_src;

	// ShadowCaster
	SHADER_LOAD_BEGIN()

	frag_src= global::File::readText("shaders/shadowcast.frag");
	vert_src= global::File::readText("shaders/shadowcast.vert");


	shadowCasterShaders.pushBack(visual::Shader());
	shadowCasterShaders.back().setSources(vert_src.cStr(), "", frag_src.cStr());
	shadowCasterShaders.back().compile<visual::Vertex>();
	SHADER_LOAD_END("ShadowCaster")

	// ShadowMap
	SHADER_LOAD_BEGIN()
	frag_src= global::File::readText("shaders/shadowmap.frag");
	vert_src= global::File::readText("shaders/shadowmap.vert");

	shadowMapShaders.reserve(7);

	for (int32 i=0; i<7; i++){
		shadowMapShaders.pushBack(visual::Shader());
		shadowMapShaders.back().setSources(vert_src.cStr(), "", frag_src.cStr());
		shadowMapShaders.back().setDefine("STATE", i);
		shadowMapShaders.back().compile<visual::Vertex>();
	}
	SHADER_LOAD_END("ShadowMap")

	// Particle
	SHADER_LOAD_BEGIN()
	frag_src= global::File::readText("shaders/particle.frag");
	vert_src= global::File::readText("shaders/particle.vert");
	geom_src= global::File::readText("shaders/particle.geom");

	particleShaders.pushBack(visual::Shader());
	particleShaders.back().setSources(vert_src, geom_src, frag_src);
	particleShaders.back().compile<visual::ParticleVBOVertexData>();
	SHADER_LOAD_END("Particle")
}


void Cache::createGenericShader(GenericShaderType t){
	print(debug::Ch::Resources, debug::Vb::Trivial,		"createGenericShader(..): \n"
											"	lightCount: %i\n"
											"	colorMap: %i\n"
											"	normalMap: %i\n"
											"	envShadowMap: %i\n",
											t.lightCount,
											t.colorMap,
											t.normalMap,
											t.envShadowMap);

	auto &shd= genericShaderMap[t];
	ensure(!shd.isCompiled());

	shd.setSources(		genericShaderVertSrc.cStr(),
						genericShaderGeomSrc.cStr(),
						genericShaderFragSrc.cStr());


	if (t.colorMap)
		shd.setDefine("COLORMAP");

	if (t.envShadowMap)
		shd.setDefine("ENVSHADOWMAP");

	if (t.normalMap)
		shd.setDefine("NORMALMAP");

	if (t.lightCount){
		shd.setDefine("DYNAMIC_LIGHTING");
		shd.setDefine("LIGHT_COUNT", t.lightCount);
	}

	if (t.curve){
		shd.setDefine("CURVE");
	}

	if (t.sway){
		shd.setDefine("SWAY");
	}
	
	SHADER_LOAD_BEGIN()

	shd.compile<visual::Vertex>();
	
	SHADER_LOAD_END("Generic")
}

const util::DynArray<Resource*>& Cache::getResources() const {
	static util::DynArray<Resource*> all_resources;
	all_resources.clear();

	/// @todo Don't generate this every time
	for (auto& sub : subCaches){
		auto& sub_resources= sub.second->getGenericResources();
		for (auto& res : sub_resources){
			all_resources.pushBack(res);
		}
	}
	
	return all_resources;
}

void Cache::parseResource(const SerializedResource& serialized){
	for (auto& m : subCaches){
		auto& sub_cache= m.second;
		if (serialized.getTypeName() == sub_cache->getResourceTypeName()){
			sub_cache->parseResource(serialized);
			return;
		}
	}
	throw ResourceException("Invalid resource type name: %s", serialized.getTypeName().cStr());
}

util::DynArray<visual::Shader>& Cache::getShaders(ShaderId id){
	switch(id){
		case Shader_Generic:
			ensure(0);

		case Shader_ShadowCaster:
			return shadowCasterShaders;

		case Shader_ShadowMap:
			return shadowMapShaders;

		case Shader_Particle:
			return particleShaders;

		default:;
	}

	throw global::Exception("ResourceMgr::getShaders(..): id not found: %i", id);
}

visual::Shader& Cache::getGenericShader(GenericShaderType t){
	PROFILE_("resources");

	auto it= genericShaderMap.find(t);

	if (it == genericShaderMap.end()){
		createGenericShader(t);
		return genericShaderMap[t];
	}

	return it->second;
}

uint32 Cache::getShaderCount(){
	return shadowCasterShaders.size() + shadowMapShaders.size() + particleShaders.size() + genericShaderMap.size();
}

visual::Font& Cache::getFont(const util::Str8& s){

	return visual::gFontMgr->getFont(s);
}

template <typename T>
void Cache::createSubCache(){
	util::Str8 section= ResourceTraits<T>::typeName();
	BaseSubCache* cache= new typename ResourceTraits<T>::SubCacheType();
	subCaches[section]= std::move(std::unique_ptr<BaseSubCache>(cache));
}

void Cache::parseResourceFile(const ResourceFilePath& path){
	print(debug::Ch::Resources, debug::Vb::Trivial, "Parsing resource file: %s", path.fromRoot().cStr());
	
	try {
	
		util::ObjectNode root;
		root.parseText(global::File::readText(path.fromRoot()));

		processJson(root, path);

	}
	catch(const ResourceException& e){
		print(debug::Ch::Resources, debug::Vb::Critical, "Cache::parseResourceFile(..): Error during resource parsing %s: %s",
			path.fromRoot().cStr(), e.what());
	}
	catch(const global::File::Error& e){
		print(debug::Ch::Resources, debug::Vb::Critical, "Cache::parseResourceFile(..): Error reading file %s: %s",
			path.fromRoot().cStr(), e.what());
	}
}

void Cache::processJson(util::ObjectNode& root, const ResourceFilePath& path_to_resource_file){
	if (!root.isObject())
		throw ResourceException("Json root is not an object");

	util::ObjectNode::MemberNames section_names=  root.getMemberNames();

	for (uint32 i=0; i<section_names.size(); ++i){
		util::ObjectNode res_section= root.get(section_names[i]);

		if (!res_section.isArray())
			throw ResourceException("Resource section is not an array");

		auto it= subCaches.find(section_names[i].cStr());
		if (it == subCaches.end()){
			print(debug::Ch::Resources, debug::Vb::Critical, "Invalid name %s for a resource section", section_names[i].cStr());
			continue;
		}
		
		print(debug::Ch::Resources, debug::Vb::Trivial, "Parsing section %s in %s",
			section_names[i].cStr(),
			util::Str8(getResourceRootPath() + path_to_resource_file.fromRoot()).cStr());
		
		it->second->parseResourceSection(res_section, path_to_resource_file);
	}
}

} // resources
} // clover
