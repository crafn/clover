#ifndef CLOVER_GAME_REGIONDATA_HPP
#define CLOVER_GAME_REGIONDATA_HPP

#include "build.hpp"
#include "worldchunk.hpp"
#include "worldgrid.hpp"

namespace clover {
namespace game {

/// Contains serialized data of a region i.e. data of one file
class RegionData {
public:
	using Id= SizeType;
	using DataRequest= std::function<util::DynArray<uint8> (Id)>;
	
	RegionData(Id id, RegionVec pos, DataRequest data_request);
	Id getId() const { return id; }
	RegionVec getPosition() const { return position; }

	void serializeChunk(const game::WorldChunk& ch);
	util::DynArray<uint8> getEntityData(ChunkVec pos) const;

	util::DynArray<uint8> getData() const;

	void clear();

private:
	void loadData(util::DynArray<uint8> data) const;
	void tryLoad() const;
	
	using EntityData= util::DynArray<uint8>;
	
	Id id;
	RegionVec position;
	DataRequest dataRequest;
	mutable bool loaded= false;
	mutable util::Map<ChunkVec, EntityData> chunks;
};

} // game
} // clover

#endif // CLOVER_GAME_REGIONDATA_HPP