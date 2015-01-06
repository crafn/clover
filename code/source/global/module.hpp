#ifndef CLOVER_GLOBAL_MODULE_HPP
#define CLOVER_GLOBAL_MODULE_HPP

#include "build.hpp"
#include "hardware/dll.hpp"
#include "resources/resource.hpp"

namespace clover {
namespace global {

class Module;

} // global
namespace resources {

template <>
struct ResourceTraits<global::Module> {
	DECLARE_RESOURCE_TRAITS(global::Module, String)
	
	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"),
							AttributeDef::Path("file"));

	typedef SubCache<global::Module> SubCacheType;

	static util::Str8 typeName(){ return "Module"; }
	static util::Str8 identifierKey(){ return "name"; }

	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static bool expired(const global::Module& m){ return false; }
};

} // resources
namespace global {

class Module : public resources::Resource {
public:
	DECLARE_RESOURCE(Module)

	Module();
	~Module();

	void* getSym(const char* name) const;

	virtual void resourceUpdate(bool load, bool force=false) override;
	virtual void createErrorResource() override;

private:
	void clear();

	RESOURCE_ATTRIBUTE(String, name);
	RESOURCE_ATTRIBUTE(Path, file);

	hardware::DllHandle dll;
};


} // global
} // clover

#endif // CLOVER_GLOBAL_MODULE_HPP
