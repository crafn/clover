#ifndef CLOVER_SCRIPT_MODULESUBCACHE_HPP
#define CLOVER_SCRIPT_MODULESUBCACHE_HPP

#include "build.hpp"
#include "module.hpp"
#include "resources/subcache.hpp"

namespace clover {
namespace script {

class ModuleSubCache : public resources::SubCache<Module> {
public:
	typedef resources::SubCache<Module> BaseType;
	
	virtual ~ModuleSubCache();
	
	virtual void preLoad();
	
private:
	void registerEverything();
};

} // script
} // clover

#endif // CLOVER_SCRIPT_MODULESUBCACHE_HPP