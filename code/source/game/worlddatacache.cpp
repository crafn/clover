#include "worlddatacache.hpp"

namespace clover {
namespace game {

RegionData& WorldDataCache::getRegion(RegionVec pos){
	auto it= regions.find(pos);
	if (it != regions.end()){
		return *it->second;
	}
	else {
		if (!util::contains(regionPositions, pos))
			regionPositions.pushBack(pos);
		auto region= util::UniquePtr<RegionData>(new RegionData(getRegionId(pos), pos, regionDataRequest));
		regions.insert(std::make_pair(pos, std::move(region)));
		return getRegion(pos);
	}
}

WorldDataCache::RegionId WorldDataCache::getRegionId(RegionVec pos) const {
	RegionId id= 0;
	for (auto& p : regionPositions){
		if (p == pos)
			return id;
		++id;
	}
	release_ensure_msg(0, "Region not found: (%i, %i)", (int)pos.x, (int)pos.y);
}

void WorldDataCache::removeRegionData(RegionVec pos){
	ensure(regions.find(pos) != regions.end());
	regions.erase(regions.find(pos));
}

util::Set<RegionVec> WorldDataCache::getCachedRegionPositions() const {
	util::Set<RegionVec> set;
	for (const auto& pair : regions){
		set.insert(pair.first);
	}
	return set;
}

void WorldDataCache::setRegionPositions(util::DynArray<RegionVec> positions){
	regionPositions= positions;
}

util::DynArray<RegionVec> WorldDataCache::getRegionPositions() const {
	return regionPositions;
}

void WorldDataCache::updateClusters(ClusterSet upd_clusters){
	util::Set<ChunkVec> upd_chunks;
	for (auto& c : upd_clusters){
		upd_chunks.insert(c.begin(), c.end());
	}
	
	// Remove updated chunks from old clusters
	ClusterSet new_clusters;
	for (auto& old_cluster : clusters){
		new_clusters.insert(util::removed(old_cluster, upd_chunks));
	}
	
	// Add updated clusters
	for (auto& upd_cluster : upd_clusters){
		new_clusters.insert(upd_cluster);
	}
	
	// Remove empty cluster
	auto it= new_clusters.find(ChunkSet{});
	if (it != new_clusters.end())
		new_clusters.erase(it);
	
	clusters= std::move(new_clusters);
}

WorldDataCache::ClusterSet WorldDataCache::getClusters() const {
	return clusters;
}

WorldDataCache::ClusterSet WorldDataCache::getClustersIncluding(ChunkSet chunks) const {
	return util::removedIf(getClusters(), [&chunks](ChunkSet chs){ return !util::overlaps(chs, chunks); });
}

} // game
} // clover