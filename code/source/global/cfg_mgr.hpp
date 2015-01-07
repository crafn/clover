#ifndef CLOVER_GLOBAL_CFG_MGR_HPP
#define CLOVER_GLOBAL_CFG_MGR_HPP

#include "build.hpp"
#include "cvar.hpp"
#include "exception.hpp"
#include "util/hashmap.hpp"
#include "util/string.hpp"

namespace clover {
namespace global {

/// Manages global configuration variables
/// @todo Rename to `Cfg` to match `g_env.cfg`
class CfgMgr {
public:
	CfgMgr();
	~CfgMgr();
	
	CfgMgr(const CfgMgr&)= delete;
	CfgMgr(CfgMgr&&)= delete;
	
	CfgMgr& operator=(const CfgMgr&)= delete;
	CfgMgr& operator=(CfgMgr&&)= delete;
	
	/// Deserializes and returns variable from cfg
	/// Throws global::Exception if something goes wrong
	template <typename T>
	T get(const util::Str8& name) const;
	
	/// Same as get(name), but instead of throwing returns value_on_error when something goes wrong
	template <typename T>
	T get(const util::Str8& name, const T& value_on_error) const;
	
	/// Loads settings from file
	void load();
	
	/// Saves settings to file
	void save();
	
private:
	
	void createVars();
	/// All possible paths where cfg file can be found
	static util::DynArray<util::Str8> defaultCfgFilePaths(){ return { "config.cfg", "../../resources/config_default.cfg" }; }
	/// Returns found config file, or if none found, path closest to binary
	util::Str8 findCfgFilePath() const;

	/// Key: "namespace::nameOfCVar"
	util::HashMap<util::Str8, CVar> vars;
	/// Contains values of all CVars. CVars have references to this tree
	util::ObjectNode root; 
};

template <typename T>
T CfgMgr::get(const util::Str8& name) const {
	auto it= vars.find(name);
	if (it == vars.end())
		throw global::Exception("Couldn't find CVar %s", name.cStr());
		
	return it->second.get<T>();
}

template <typename T>
T CfgMgr::get(const util::Str8& name, const T& value_on_error) const {
	try {
		return get<T>(name);
	}
	catch (global::Exception& e){
		return value_on_error;
	}
}

} // global
namespace util {

template <>
struct TypeStringTraits<global::CfgMgr> {
	static util::Str8 type(){ return "::CfgMgr"; }
};

} // util
} // clover

#endif // CLOVER_GLOBAL_CFG_MGR_HPP
