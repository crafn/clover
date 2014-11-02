#ifndef CLOVER_GAME_WORLD_GEN_WORKERTYPE_HPP
#define CLOVER_GAME_WORLD_GEN_WORKERTYPE_HPP

#include "build.hpp"
#include "resources/resource.hpp"
#include "script/function.hpp"
#include "script/module.hpp"

namespace clover {
namespace game { namespace world_gen {

class ChunkGen;
class WorldGen;
class WorkerLocation;
class WorkerType;
	
}} // game::world_gen
namespace resources {

template <>
struct ResourceTraits<game::world_gen::WorkerType> {
	DECLARE_RESOURCE_TRAITS(game::world_gen::WorkerType, String)
	
	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"),
							AttributeDef::Resource("scriptModule"),
							AttributeDef::String("globalInitFunc"),
							AttributeDef::String("chunkInitFunct"),
							AttributeDef::String("workFunc"));
	
	typedef SubCache<game::world_gen::WorkerType> SubCacheType;
	
	static util::Str8 typeName(){ return "WorldGenWorkerType"; }
	static util::Str8 identifierKey(){ return "name"; }
	
	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static bool expired(const game::world_gen::WorkerType& w){ return false; }
};

} // resources
namespace game { namespace world_gen {

using WorkerGlobalInitFuncDecl= void (WorldGen&);
using WorkerChunkInitFuncDecl= void (ChunkGen&);
using WorkFuncDecl= void (WorldGen&, const WorkerLocation&);

/// Defines type of a world generation Worker
/// Contains information of worker initialization and work-function
class WorkerType : public resources::Resource {
public:
	DECLARE_RESOURCE(WorkerType)
	
	WorkerType();
	
	/// Generation functions
	void callGlobalInit(WorldGen& w) const;
	void callChunkInit(ChunkGen& c) const;
	void callWork(WorldGen& w, const WorkerLocation& loc) const;
	
	virtual void resourceUpdate(bool load, bool force=false) override;
	virtual void createErrorResource() override;
	
private:
	void updateFromAttributes();
	void clear();

	const script::Module* module;
	std::function<WorkerGlobalInitFuncDecl> globalInitFunc;
	std::function<WorkerChunkInitFuncDecl> chunkInitFunc;
	std::function<WorkFuncDecl> workFunc;

	RESOURCE_ATTRIBUTE(String, nameAttribute);
	RESOURCE_ATTRIBUTE(Resource, scriptModuleAttribute);
	RESOURCE_ATTRIBUTE(String, globalInitFuncAttribute);
	RESOURCE_ATTRIBUTE(String, chunkInitFuncAttribute);
	RESOURCE_ATTRIBUTE(String, workFuncAttribute);
};

}} // game::world_gen
} // clover

#endif // CLOVER_GAME_WORLD_GEN_WORKERTYPE_HPP