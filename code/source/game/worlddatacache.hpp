#ifndef CLOVER_GAME_WORLDDATACACHE_HPP
#define CLOVER_GAME_WORLDDATACACHE_HPP

#include "build.hpp"
#include "regiondata.hpp"
#include "worldgrid.hpp"

namespace clover {
namespace game {

/// Data cache for serialized world regions
class WorldDataCache {
public:
	using RegionId= RegionData::Id;
	using RegionDataRequest= RegionData::DataRequest;
	using ChunkSet= util::Set<ChunkVec>;
	using ClusterSet= util::Set<ChunkSet>;
	
	/// @param region_request 	is called when data for new region is needed.
	///							Will be called in the deserialization of chunks
	WorldDataCache(RegionDataRequest region_request)
			: regionDataRequest(region_request){}
	
	RegionData& getRegion(RegionVec pos);
	void touchRegion(RegionVec pos){ getRegion(pos); }
	void removeRegionData(RegionVec pos);
	
	/// Doesn't get/touch region
	RegionId getRegionId(RegionVec pos) const;
	
	util::Set<RegionVec> getCachedRegionPositions() const;
	
	void setRegionPositions(util::DynArray<RegionVec> p);
	util::DynArray<RegionVec> getRegionPositions() const;
	
	void updateClusters(ClusterSet clusters);
	ClusterSet getClusters() const;
	ClusterSet getClustersIncluding(ChunkSet chunks) const;
	
private:
	/// @todo	If multiple tasks are allowed to run simultaneously,
	///			regions should be atomic
	util::Map<RegionVec, util::UniquePtr<RegionData>> regions;
	/// All regions ever created (even in save)
	util::DynArray<RegionVec> regionPositions;
	ClusterSet clusters;
	
	RegionDataRequest regionDataRequest;
};

} // game
} // clover

#endif // CLOVER_GAME_WORLDDATACACHE_HPP