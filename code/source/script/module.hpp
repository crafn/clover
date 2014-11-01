#ifndef CLOVER_SCRIPT_MODULE_HPP
#define CLOVER_SCRIPT_MODULE_HPP

#include "build.hpp"
#include "resources/resource.hpp"
//.tpp
#include "function.hpp"
#include "typestring.hpp"
#include "exception.hpp"

//.tpp
#include <angelscript.h>

class asIScriptModule;

namespace clover {
namespace script {

class Module;
class ModuleSubCache;

} // script
namespace resources {

template <>
struct ResourceTraits<script::Module> {
	DECLARE_RESOURCE_TRAITS(script::Module, String)
	
	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"),
							AttributeDef::Path("file"))
	
	typedef script::ModuleSubCache SubCacheType;

	static util::Str8 typeName(){ return "ScriptModule"; }
	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static util::Str8 identifierKey(){ return "name"; }

	static bool expired(const script::Module&){ return false; }
	

};

} // resources
namespace script {

class ObjectType;

template<typename F>
class Function;

/// A bunch of scripts which know about each other
/// Can't be used if in Unloaded state
class Module : public resources::Resource {
public:
	DECLARE_RESOURCE(Module)
	
	Module();
	virtual ~Module();
	
	virtual void resourceUpdate(bool load, bool force= true);
	virtual void createErrorResource();
	
	util::Str8 getName() const { return nameAttribute.get(); }
	
	template <typename F>
	Function<F> getGlobalFunction(const util::Str8& name) const;
	ObjectType getObjectType(const util::Str8& name) const;

	asIScriptModule& getAsModule() const;
	
private:
	void create();
	void build();
	void destroy();
	
	//util::DynArray<util::Str8> filePaths;
	
	asIScriptModule* module;
	
	RESOURCE_ATTRIBUTE(String, nameAttribute)
	RESOURCE_ATTRIBUTE(Path, fileAttribute)
};

#include "module.tpp"

} // script
} // clover

#include "modulesubcache.hpp"

#endif // CLOVER_SCRIPT_MODULE_HPP