#include "../world_mgr.hpp"
#include "chunk_gen.hpp"
#include "nodes/signaltypetraits.hpp"
#include "script.hpp"
#include "script/script_mgr.hpp"
#include "world_gen.hpp"

namespace clover {
namespace game { namespace world_gen {

using script::gScriptMgr;

template <typename T>
struct RegisterCreateEntityT {
	static void invoke(){
		gScriptMgr->registerMethod(
			static_cast<game::WeHandle (WorldGen::*)(const util::Str8& name, T t)>(&WorldGen::createEntity<T>),
			"createEntity");
	}
};

template <typename T>
struct RegisterSetAttribute {
	static void invoke(){
		script::gScriptMgr->registerMethod(&game::WorldEntity::setAttribute<T>, "setAttribute");
	}
};

void registerToScript(){
	nodes::callForEverySignalValueType<RegisterSetAttribute>();

	gScriptMgr->registerObjectType<WorldGen*>();
	gScriptMgr->registerMethod(&WorldGen::getWorldMgr, "getWorldMgr");

	gScriptMgr->registerMethod(static_cast<game::WeHandle (WorldGen::*)(const util::Str8& name)>(&WorldGen::createEntity), "createEntity");
	gScriptMgr->registerMethod(static_cast<game::WeHandle (WorldGen::*)(const util::Str8& name, util::Vec2d t)>(&WorldGen::createEntity), "createEntity");
	gScriptMgr->registerMethod(static_cast<game::WeHandle (WorldGen::*)(const util::Str8& name, util::Vec3d t)>(&WorldGen::createEntity), "createEntity");
	RegisterCreateEntityT<util::RtTransform2d>::invoke();
	RegisterCreateEntityT<util::SrtTransform2d>::invoke();
	RegisterCreateEntityT<util::SrtTransform3d>::invoke();
	
	gScriptMgr->registerMethod(&WorldGen::createWorker, "createWorker");
	
	gScriptMgr->registerObjectType<ChunkGen*>();
	gScriptMgr->registerMethod(&ChunkGen::getWorldGen, "getWorldGen");
	gScriptMgr->registerMethod(&ChunkGen::createEntity, "createEntity");
	gScriptMgr->registerMethod(&ChunkGen::getPosition, "getPosition");
	gScriptMgr->registerMethod(&ChunkGen::createWorker, "createWorker");
	
	gScriptMgr->registerObjectType<WorkerLocation>();
	gScriptMgr->registerMethod(&WorkerLocation::getPosition, "getPosition");
	gScriptMgr->registerMethod(&WorkerLocation::getTime, "getTime");
	gScriptMgr->registerMethod(&WorkerLocation::getChunkGen, "getChunkGen");
}

}} // game::world_gen
} // clover
