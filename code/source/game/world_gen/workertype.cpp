#include "workertype.hpp"
#include "chunk_gen.hpp"
#include "hardware/dll.hpp"
#include "resources/cache.hpp"
#include "world_gen.hpp"
#include "workerlocation.hpp"

namespace clover {
namespace game { namespace world_gen {

WorkerType::WorkerType()
		: INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(moduleAttribute, "module", "")
		, INIT_RESOURCE_ATTRIBUTE(globalInitFuncAttribute, "globalInitFunc", "")
		, INIT_RESOURCE_ATTRIBUTE(chunkInitFuncAttribute, "chunkInitFunc", "")
		, INIT_RESOURCE_ATTRIBUTE(workFuncAttribute, "workFunc", ""){
	
	auto try_refresh= [this] (){
		if (getResourceState() != State::Uninit)
			resourceUpdate(false);
	};
	
	moduleAttribute.setOnChangeCallback(try_refresh);
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

	/// TEMPTEST
	auto h= hardware::loadDll("./mod");
	ensure_msg(h, "dll not loaded: %s", hardware::dllError());
	if (!globalInitFuncAttribute.get().empty()){
		auto func= (WorkerGlobalInitFuncDecl*)
			hardware::queryDllSym(h, globalInitFuncAttribute.get().cStr());
		ensure(func);
		globalInitFunc= [this, func] (WorldGen& g){
			func(g);
		};
	}

	if (!chunkInitFuncAttribute.get().empty()){
		auto func= (WorkerChunkInitFuncDecl*)
			hardware::queryDllSym(h, chunkInitFuncAttribute.get().cStr());
		ensure_msg(func, "Chunk func not found: %s", chunkInitFuncAttribute.get().cStr());
		chunkInitFunc= [this, func] (ChunkGen& c){
			func(c);
		};
	}

	if (!workFuncAttribute.get().empty()){
		auto func= (WorkFuncDecl*)
			hardware::queryDllSym(h, workFuncAttribute.get().cStr());
		ensure(func);
		workFunc= [this, func] (WorldGen& g, const Worker* w){
			func(g, w);
		};
	}
}

void WorkerType::clear(){
	globalInitFunc= nullptr;
	chunkInitFunc= nullptr;
	workFunc= nullptr;
}

}} // game::world_gen
} // clover
