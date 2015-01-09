#include "chunk_gen.hpp"
#include "global/module.hpp"
#include "hardware/dll.hpp"
#include "resources/cache.hpp"
#include "world_gen.hpp"
#include "workerlocation.hpp"
#include "workertype.hpp"

namespace clover {
namespace game { namespace world_gen {

WorkerType::WorkerType()
		: INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(moduleAttribute, "module", "")
		, INIT_RESOURCE_ATTRIBUTE(globalInitFuncAttribute, "globalInitFunc", "")
		, INIT_RESOURCE_ATTRIBUTE(chunkInitFuncAttribute, "chunkInitFunc", "")
		, INIT_RESOURCE_ATTRIBUTE(workFuncAttribute, "workFunc", "")
{

	auto try_refresh= [this] ()
	{
		if (getResourceState() != State::Uninit)
			resourceUpdate(false);
	};

	moduleAttribute.setOnChangeCallback(try_refresh);
	globalInitFuncAttribute.setOnChangeCallback(try_refresh);
	chunkInitFuncAttribute.setOnChangeCallback(try_refresh);
	workFuncAttribute.setOnChangeCallback(try_refresh);
}

void WorkerType::callGlobalInit(WorldGen& gen) const
{
	if (globalInitFunc)
		globalInitFunc(gen);
}

void WorkerType::callChunkInit(ChunkGen& gen) const
{
	if (chunkInitFunc)
		chunkInitFunc(gen);
}

void WorkerType::callWork(WorldGen& gen, const Worker& w) const
{
	/// @todo Return false if no work was done -- important to handle at reloading
	if (workFunc)
		workFunc(gen, &w);
}

void WorkerType::resourceUpdate(bool load, bool force)
{
	if (getResourceState() == State::Uninit || load) {
		try {
			updateFromAttributes();
			setResourceState(State::Loaded);	
		}
		catch (resources::ResourceException&) {
			createErrorResource();
		}
	} else {
		clear();
		setResourceState(State::Unloaded);
	}
}

void WorkerType::createErrorResource()
{
	clear();
	setResourceState(State::Error);
}

void WorkerType::updateFromAttributes()
{
	clear();

	auto& mod=
		global::g_env.resCache->getResource<global::Module>(
				moduleAttribute.get());

	moduleChangeListener.clear();
	moduleChangeListener.listen(mod, [this] ()
	{ resourceUpdate(false); }); // Reload on change

	if (!globalInitFuncAttribute.get().empty()) {
		auto fname= globalInitFuncAttribute.get().cStr();
		auto func= (WorkerGlobalInitFuncDecl*)mod.getSym(fname);
		if (!func)
			throw resources::ResourceException("Func not found: %s", fname);
		globalInitFunc= [this, func] (WorldGen& g){ func(g); };
	}

	if (!chunkInitFuncAttribute.get().empty()) {
		auto fname= chunkInitFuncAttribute.get().cStr();
		auto func= (WorkerChunkInitFuncDecl*)mod.getSym(fname);
		if (!func)
			throw resources::ResourceException("Func not found: %s", fname);
		chunkInitFunc= [this, func] (ChunkGen& c){ func(c); };
	}

	if (!workFuncAttribute.get().empty()) {
		auto fname= workFuncAttribute.get().cStr();
		auto func= (WorkFuncDecl*)mod.getSym(fname);
		if (!func)
			throw resources::ResourceException("Func not found: %s", fname);
		workFunc= [this, func] (WorldGen& g, const Worker* w){ func(g, w); };
	}
}

void WorkerType::clear()
{
	globalInitFunc= nullptr;
	chunkInitFunc= nullptr;
	workFunc= nullptr;
}

}} // game::world_gen
} // clover
