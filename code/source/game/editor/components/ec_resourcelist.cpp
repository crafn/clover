#include "ec_resourcelist.hpp"
#include "global/env.hpp"
#include "resources/cache.hpp"
#include "resources/resource.hpp"

namespace clover {
namespace game { namespace editor {

util::DynArray<resources::Resource*> ResourceListEc::search(const util::Str8& search_str, uint32 max_results, bool use_tags){
	util::DynArray<util::Str8> keywords= search_str.splitted(' ');
	util::DynArray< util::DynArray<resources::Resource*> > hits_by_count(keywords.size());
	util::DynArray<resources::Resource*> resources= global::g_env.resCache->getResources();
	
	for (auto& res : resources){
		
		util::Str8 res_name= res->getIdentifierAsString();
		if (use_tags)
			res_name = "(" + res->getResourceTypeName() + ") " + " " + res_name;
		
		uint32 hit_count=0;
		for (auto& keyword : keywords){
			if (res_name.contains(keyword, false)){
				++hit_count;
			}
		}
		
		if (hit_count > 0)
			hits_by_count[hit_count-1].pushBack(res);
		
	}
	
	util::DynArray<resources::Resource*> ret;
	// Most hits first
	for (auto it= hits_by_count.rBegin(); it != hits_by_count.rEnd(); ++it){
		for (auto& m : *it){
			ret.pushBack(m);
			if (ret.size() >= max_results) break;
		}
		if (ret.size() >= max_results) break;
	}	
	return ret;
}

}} // game::editor
} // clover
