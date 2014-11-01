#ifndef CLOVER_GAME_WORLDENTITY_PACK_HPP
#define CLOVER_GAME_WORLDENTITY_PACK_HPP

#include "build.hpp"
#include "util/pack.hpp"

namespace clover {
namespace game {

/// Serialization of one object
/// @todo Remove and use some common binary serialization
class WEPack {
public:
	WEPack();
	
	void reset(uint64 id);
	
	void set(util::RawPack& data);
	
	uint64 getId(){ return id; }
	
	uint32 getVarBlockSize(){ return data.size(); }
	
	void prepareVarBlockReading();
	
	util::RawPack& getVarPack();
	
protected:
	uint64 id; // Id of WorldEntity
	util::RawPack data;
};

} // game
} // clover

#endif // CLOVER_GAME_WORLDENTITY_PACK_HPP