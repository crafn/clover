#ifndef CLOVER_GAME_WORLD_GEN_WORKERLOCATION_HPP
#define CLOVER_GAME_WORLD_GEN_WORKERLOCATION_HPP

#include "build.hpp"
#include "script/typestring.hpp"
#include "util/vector.hpp"

namespace clover {
namespace game { namespace world_gen {

class ChunkGen;

class WorkerLocation {
public:
	WorkerLocation()= default;
	WorkerLocation(WorldVec pos, real64 t, ChunkGen* c)
		: position(pos), time(t), chunkGen(c){}
	
	WorldVec getPosition() const { return position; }
	real64 getTime() const { return time; }
	ChunkGen& getChunkGen() const { ensure(chunkGen); return *chunkGen; }
	
private:
	friend class ChunkGen;
	WorldVec position;
	real64 time= 0.0;
	ChunkGen* chunkGen= nullptr;
};

}} // game::world_gen
namespace util {

template <>
struct TypeStringTraits<game::world_gen::WorkerLocation>{
	static util::Str8 type(){ return "world_gen::WorkerLocation"; }
};

} // util
} // clover

#endif // CLOVER_GAME_WORLD_GEN_WORKERLOCATION_HPP
