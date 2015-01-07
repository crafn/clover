#ifndef CLOVER_RESOURCES_RESOURCE_REF_HPP
#define CLOVER_RESOURCES_RESOURCE_REF_HPP

#include "build.hpp"
#include "global/env.hpp"
#include "resource.hpp"
#include "resources/cache.hpp"

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
	
	ResourceRef()
			: cacheRes(nullptr){
	}
	
	ResourceRef(const Res* cache_res)
			: cacheRes(cache_res){
	}
	
	ResourceRef(const Identifier& id)
			: cacheRes(&global::g_env.resCache->getResource<Res>(id)){
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
	void serialize(Archive& ar, uint32 version) {
		if (Archive::is_saving::value) {
			bool in_cache= cacheRes != nullptr;
			ar & in_cache;
			if (in_cache){
				// Need only to save identifier of a cached resource
				auto id= cacheRes->getIdentifier();
				ar & id;
			} else {
				ensure(res);
				ar & *res;
			}
		} else {
			ensure(!res);
			bool in_cache;
			ar & in_cache;
			if (in_cache){
				Identifier id;
				ar & id;
				cacheRes= &global::g_env.resCache->getResource<Res>(id);
			} else {
				res= newRes();
				ar & *res;
			}
		}
	}

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

} // resources
} // clover

#endif // CLOVER_RESOURCES_RESOURCE_REF_HPP
