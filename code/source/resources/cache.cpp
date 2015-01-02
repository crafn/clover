#include "animation/bvh_util.hpp"
#include "cache.hpp"
#include "global/env.hpp"
#include "hardware/device.hpp"
#include "visual/font_mgr.hpp"
#include "global/file.hpp"
#define RESOURCE_HEADERS
#include "resourcetypes.def"
#undef RESOURCE_HEADERS

#include <boost/filesystem.hpp>
#include <fstream>
#include <json/json.h>
#include <json/writer.h>
#if OS == OS_WINDOWS
#include <io.h>
#endif // OS == OS_WINDOWS
#include <sys/types.h>
#include <sys/stat.h>

namespace fs = boost::filesystem;

namespace clover {
namespace resources {

Cache::Cache()
	: resourcePath(DEFAULT_RES_PATH)
{
	PROFILE_("resources");
	if (!global::g_env.resCache)
		global::g_env.resCache= this;

	/// @todo Use config
	bool exists= false;
	if ( access( DEFAULT_RES_PATH2, 0 ) == 0 ){
		struct stat status;
		stat( DEFAULT_RES_PATH2, &status );

		/// @todo Fix directory checking, didn't work on windows
		exists= true;
	}
	if (exists) resourcePath= DEFAULT_RES_PATH2;

	print(debug::Ch::Resources, debug::Vb::Trivial, "Resource path: %s", resourcePath.cStr());

#define RESOURCE(type_name, auto_preload) \
	createSubCache<type_name>();
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
			}
		}
	}
	catch (const fs::filesystem_error& e){
		print(debug::Ch::Resources, debug::Vb::Critical, "Filesystem error: %s", e.what());
	}

#define RESOURCE(type_name, auto_preload) \
	if (auto_preload) preLoad<type_name>();
#include "resourcetypes.def"
#undef RESOURCE

	visual::gFontMgr= new visual::FontMgr();
	print(debug::Ch::Resources, debug::Vb::Trivial, "Loading fonts");
	loadFonts();

}

Cache::~Cache()
{
	print(debug::Ch::Resources, debug::Vb::Trivial, "Freeing resources");

	delete visual::gFontMgr;
	visual::gFontMgr= nullptr;

	// Reverse unload
	util::DynArray<std::function<void ()>> unloads;
#define RESOURCE(type_name, auto_preload) \
	if (auto_preload) unloads.pushBack([this]() { unload<type_name>(); });
#include "resourcetypes.def"
#undef RESOURCE
	for (SizeType i= unloads.size(); i > 0; --i)
		unloads[i - 1]();

	resourceFileWatchers.clear();
	resourceFilePaths.clear();
	subCaches.clear();

	if (global::g_env.resCache == this)
		global::g_env.resCache= nullptr;
}

void Cache::update()
{
	PROFILE_("resources");
	for (auto& m : subCaches){
		m.second->update();
	}
}

const Resource& Cache::getResource(const ResourceId& res_id)
{
	ensure(subCaches.find(res_id.getTypeName()) != subCaches.end());
	return subCaches[res_id.getTypeName()]->getResource(res_id.getIdentifier());
}

void Cache::writeAllResources()
{
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

void Cache::loadFonts()
{
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

const util::DynArray<Resource*>& Cache::getResources() const
{
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

void Cache::parseResource(const SerializedResource& serialized)
{
	for (auto& m : subCaches){
		auto& sub_cache= m.second;
		if (serialized.getTypeName() == sub_cache->getResourceTypeName()){
			sub_cache->parseResource(serialized);
			return;
		}
	}
	throw ResourceException("Invalid resource type name: %s", serialized.getTypeName().cStr());
}

visual::Font& Cache::getFont(const util::Str8& s)
{ return visual::gFontMgr->getFont(s); }

template <typename T>
void Cache::preLoad()
{
	for (auto&& path : resourceFilePaths) {
		parseResourceFile(path, ResourceTraits<T>::typeName());
	}
	getSubCache<T>().preLoad();
}

template <typename T>
void Cache::unload()
{
	auto it= subCaches.find(ResourceTraits<T>::typeName());
	ensure(it != subCaches.end());
	subCaches.erase(it);
}

template <typename T>
void Cache::createSubCache()
{
	util::Str8 section= ResourceTraits<T>::typeName();
	BaseSubCache* cache= new typename ResourceTraits<T>::SubCacheType();
	subCaches[section]= std::move(std::unique_ptr<BaseSubCache>(cache));
}

/// @todo Add every function template
#define RESOURCE(type_name, auto_preload) \
template void Cache::preLoad<type_name>(); \
template void Cache::unload<type_name>(); \
template void Cache::createSubCache<type_name>();
#include "resourcetypes.def"
#undef RESOURCE

void Cache::parseResourceFile(
		const ResourceFilePath& path,
		const util::Str8& only_this)
{
	print(debug::Ch::Resources, debug::Vb::Trivial, "Parsing resource file: %s", path.fromRoot().cStr());

	try {
		util::ObjectNode root;
		root.parseText(global::File::readText(path.fromRoot()));
		processResData(root, path, only_this);
	}
	catch(const ResourceException& e){
		print(debug::Ch::Resources, debug::Vb::Critical,
				"Cache::parseResourceFile(..): Error during resource parsing %s: %s",
				path.fromRoot().cStr(), e.what());
	}
	catch(const global::File::Error& e){
		print(debug::Ch::Resources, debug::Vb::Critical,
				"Cache::parseResourceFile(..): Error reading file %s: %s",
				path.fromRoot().cStr(), e.what());
	}
}

void Cache::processResData(
		util::ObjectNode& root,
		const ResourceFilePath& path_to_resource_file,
		const util::Str8& only_this)
{
	if (!root.isObject())
		throw ResourceException("Json root is not an object");

	util::ObjectNode::MemberNames section_names=  root.getMemberNames();

	for (uint32 i= 0; i < section_names.size(); ++i){
		if (!only_this.empty() && section_names[i] != only_this)
			continue;

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
