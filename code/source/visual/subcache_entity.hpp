#ifndef CLOVER_VISUAL_SUBCACHE_ENTITY_HPP
#define CLOVER_VISUAL_SUBCACHE_ENTITY_HPP

#include "build.hpp"
#include "entity_def.hpp"
#include "resources/subcache.hpp"

namespace clover {
namespace resources {

class VisualEntityDefSubCache : public SubCache<visual::EntityDef> {
public:
	virtual ~VisualEntityDefSubCache();
	
	/// Searches also other *EntityDef subcaches
	virtual visual::EntityDef* findResource(const util::Str8& key) override;
	virtual const visual::EntityDef& getErrorResource() override;
	
	/// Other *EntityDef subcaches call this for their entitydefs
	static void addEntityDef(const util::Str8& identifier, visual::EntityDef& entitydef);
	
private:
	typedef SubCache<visual::EntityDef> BaseType;
	virtual visual::EntityDef& createNewResource(const util::Str8& identifier);
	
	static util::HashMap<util::Str8, visual::EntityDef*> allEntityDefs;
};

/// Cache for types derived from EntityDef
template <typename T>
class VisualDerivedEntityDefSubCache : public SubCache<T> {
public:
	virtual ~VisualDerivedEntityDefSubCache(){}
private:
	typedef SubCache<T> BaseType;
	virtual T& createNewResource(const util::Str8& identifier){
		T& res= BaseType::createNewResource(identifier);
		VisualEntityDefSubCache::addEntityDef(identifier, res);
		return res;
	}
};

} // visual
} // clover

#endif // CLOVER_VISUAL_SUBCACHE_ENTITY_HPP