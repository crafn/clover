#include "workertype.hpp"
#include "chunk_gen.hpp"
#include "resources/cache.hpp"
#include "script/script_mgr.hpp"
#include "world_gen.hpp"
#include "workerlocation.hpp"

namespace clover {
namespace game { namespace world_gen {

WorkerType::WorkerType()
		: module(nullptr)
		, INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(scriptModuleAttribute, "scriptModule", "")
		, INIT_RESOURCE_ATTRIBUTE(globalInitFuncAttribute, "globalInitFunc", "")
		, INIT_RESOURCE_ATTRIBUTE(chunkInitFuncAttribute, "chunkInitFunc", "")
		, INIT_RESOURCE_ATTRIBUTE(workFuncAttribute, "workFunc", ""){
	
	auto try_refresh= [this] (){
		if (getResourceState() != State::Uninit)
			resourceUpdate(false);
	};
	
	scriptModuleAttribute.setOnChangeCallback(try_refresh);
	globalInitFuncAttribute.setOnChangeCallback(try_refresh);
	chunkInitFuncAttribute.setOnChangeCallback(try_refresh);
	workFuncAttribute.setOnChangeCallback(try_refresh);
}

void WorkerType::callGlobalInit(WorldGen& gen) const {
	if (globalInitFunc)
		globalInitFunc(gen);
}

void WorkerType::callChunkInit(ChunkGen& gen) const {
	if (chunkInitFunc)
		chunkInitFunc(gen);
}

void WorkerType::callWork(WorldGen& gen, const Worker& w) const {
	if (workFunc)
		workFunc(gen, &w);
}

void WorkerType::resourceUpdate(bool load, bool force){
	if (getResourceState() == State::Uninit || load){
		try {
			updateFromAttributes();
			setResourceState(State::Loaded);	
		}
		catch (resources::ResourceException&){
			createErrorResource();
		}
	}
	else {
		clear();
		setResourceState(State::Unloaded);
	}
}

void WorkerType::createErrorResource(){
	clear();
	setResourceState(State::Error);
}

void WorkerType::updateFromAttributes(){
	clear();
	
	module= &resources::gCache->getResource<script::Module>(scriptModuleAttribute.get());
	ensure(module);
	
	/// @todo Deuglify this code
	
	if (!globalInitFuncAttribute.get().empty()){
		auto script_func= module->getGlobalFunction<WorkerGlobalInitFuncDecl>(globalInitFuncAttribute.get());
		globalInitFunc= [this, script_func] (WorldGen& g){
			script::gScriptMgr->getFreeContext().execute(script_func(g));
		};
	}

	if (!chunkInitFuncAttribute.get().empty()){
		auto script_func= module->getGlobalFunction<WorkerChunkInitFuncDecl>(chunkInitFuncAttribute.get());
		chunkInitFunc= [this, script_func] (ChunkGen& c){
			script::gScriptMgr->getFreeContext().execute(script_func(c));
		};
	}

	if (!workFuncAttribute.get().empty()){
		auto script_func= module->getGlobalFunction<WorkFuncDecl>(workFuncAttribute.get());
		workFunc= [this, script_func] (WorldGen& g, const Worker* w){
			script::gScriptMgr->getFreeContext().execute(script_func(g, w));
		};
	}
	
}

void WorkerType::clear(){
	module= nullptr;
	globalInitFunc= nullptr;
	chunkInitFunc= nullptr;
	workFunc= nullptr;
}

}} // game::world_gen
} // clover
