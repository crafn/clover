#ifndef CLOVER_RESOURCES_RESOURCE_REF_HPP
#define CLOVER_RESOURCES_RESOURCE_REF_HPP

#include "build.hpp"
#include "resource.hpp"
#include "resources/cache.hpp"
#include "script/script_mgr.hpp"
#include "script/typestring.hpp"

#include <memory>

namespace clover {
namespace resources {

/// Can hold reference to a resource or a unique instance
/// Used to achieve uniform handling of a resource object whether it was in a cache or not
template <typename Res>
class ResourceRef {
public:
	using Identifier= typename ResourceTraits<Res>::IdentifierValue;
	using ResPtr= std::shared_ptr<Res>;
	
	static void registerToScript();
	
	ResourceRef()
			: cacheRes(nullptr){
	}
	
	ResourceRef(const Res* cache_res)
			: cacheRes(cache_res){
	}
	
	ResourceRef(const Identifier& id)
			: cacheRes(&resources::gCache->getResource<Res>(id)){
	}
	
	ResourceRef(const ResourceRef& other)
			: res(other.res)
			, cacheRes(other.cacheRes){
		
	}
	
	ResourceRef(ResourceRef&& other)
			: res(std::move(other.res))
			, cacheRes(other.cacheRes){
		other.res= nullptr;
		other.cacheRes= nullptr;
	}
	
	~ResourceRef(){
	}
	
	ResourceRef& operator=(const ResourceRef& other){
		res= other.res;
		cacheRes= other.cacheRes;
		return *this;
	}
	
	ResourceRef& operator=(ResourceRef&& other){
		res= std::move(other.res);
		cacheRes= other.cacheRes;
		other.cacheRes= nullptr;
		return *this;
	}
	
	const Res& get() const {
		if (cacheRes){
			return *cacheRes;
		}
		else {
			if (!res)
				res= newRes();
			return *res;
		}
	}
	
	/// Returns unique modifiable object
	Res& ref(){
		if (!res){
			if (cacheRes){
				// Copy cacheRes so that it can be modified
				res= copy(cacheRes);
				cacheRes= nullptr;					
			}
			else {
				res= newRes();
			}
		}
		
		ensure(res);
		return *res;
	}
	
	bool isSet() const { return cacheRes || res; }
	
	template <typename Archive>
	void save(Archive& ar, uint32 version) const {
		bool in_cache= cacheRes != nullptr;
		ar & in_cache;
		if (in_cache){
			// Need only to save identifier of a cached resource
			ar & cacheRes->getIdentifier();
		}
		else {
			ensure(res);
			ar & *res;
		}
	}
	
	template <typename Archive>
	void load(Archive& ar, uint32 version){
		ensure(!res);
		
		bool in_cache;
		ar & in_cache;
		if (in_cache){
			Identifier id;
			ar & id;
			cacheRes= &resources::gCache->getResource<Res>(id);
		}
		else {
			res= newRes();
			ar & *res;
		}
	}
	
	BOOST_SERIALIZATION_SPLIT_MEMBER()
	
private:
	static ResPtr newRes(){
		return ResPtr(new Res());
	}
	
	static ResPtr copy(const Res* src){
		if (!src)
			return nullptr;
		else
			return ResPtr(new Res(*src));
	}
	
	// Mutable for late creation
	mutable ResPtr res;
	const Res* cacheRes;
};


template <typename Res>
void ResourceRef<Res>::registerToScript(){
	static bool template_registered;
	if (!template_registered){
		script::gScriptMgr->registerTemplateType<ResourceRef>();
		template_registered= true;
	}
	
	script::gScriptMgr->registerObjectType<ResourceRef<Res>>();
	script::gScriptMgr->registerConstructor<ResourceRef<Res>, const Identifier&>();
	script::gScriptMgr->registerMethod(&ResourceRef<Res>::ref, "ref");
	script::gScriptMgr->registerMethod(&ResourceRef<Res>::get, "get");
}

} // resources
namespace util {

template <typename Res>
struct TypeStringTraits<resources::ResourceRef<Res>>{
	static util::Str8 type(){ return "::ResourceRef<" + script::TypeString<Res*>()() + ">"; }
};

} // util
} // clover

#endif // CLOVER_RESOURCES_RESOURCE_REF_HPP
