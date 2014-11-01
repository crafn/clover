#include "regiondata.hpp"

namespace clover {
namespace game {

RegionData::RegionData(Id id_, RegionVec pos, DataRequest data_request)
		: id(id_)
		, position(pos)
		, dataRequest(data_request){
}

void RegionData::serializeChunk(const game::WorldChunk& ch){
	ensure(chunks.find(ch.getPosition()) == chunks.end() || loaded);
	chunks[ch.getPosition()]= ch.getEntityData();
}

util::DynArray<uint8> RegionData::getEntityData(ChunkVec pos) const {
	tryLoad();
	
	ensure(game::GridPoint::Chunk(pos).getRegionVec() == position);
	ensure_msg(chunks.find(pos) != chunks.end(), "Entity data for chunk not found: %i, %i", (int)pos.x, (int)pos.y);
	
	return chunks.find(pos)->second;
}

util::DynArray<uint8> RegionData::getData() const {
	// If there's been only serialization chunks in file haven't been loaded yet
	tryLoad();
	ensure(loaded);
	
	util::RawPack data_pack;	
	for (const auto& pair : chunks){
		ChunkVec pos= pair.first;
		const auto& data= pair.second;
		
		data_pack << game::WorldChunk::chunkStartTag;
		data_pack << pos << (uint32)data.size();
		data_pack.addData(data.data(), data.size());
	}
	return std::move(data_pack.getData());
}

void RegionData::clear(){
	chunks.clear();
	loaded= false;
}

void RegionData::loadData(util::DynArray<uint8> data) const {
	if (data.empty())
		return;
	
	//print(debug::Ch::Save, debug::Vb::Trivial, "RegionData::parseData: (%i, %i)", (int)position.x, (int)position.y);
	util::RawPack data_pack;
	data_pack.addData(data.data(), data.size());
	data_pack.seekg(0);
	data_pack.setReadMode();

	while (!data_pack.eof()){
		uint16 tag= 0;
		data_pack >> tag;
		ensure(tag == game::WorldChunk::chunkStartTag);

		ChunkVec chunk_pos;
		uint32 chunk_data_size;

		data_pack >> chunk_pos >> chunk_data_size;
		
		ensure(game::GridPoint::Chunk(chunk_pos).getRegionVec() == position);
		ensure_msg(chunks.find(chunk_pos) == chunks.end(), "Double chunk: (%i, %i)", chunk_pos.x, chunk_pos.y);
		
		ensure(chunks.find(chunk_pos) == chunks.end());
		EntityData& d= chunks[chunk_pos];
		//print(debug::Ch::Save, debug::Vb::Trivial, "Parsing chunk data: (%i, %i)", (int)chunk_pos.x, (int)chunk_pos.y);
		
		if (chunk_data_size == 0)
			continue;
		
		d.resize(chunk_data_size);
		memcpy(&d[0], &data_pack[data_pack.tellg()], chunk_data_size);
		data_pack.seekg(data_pack.tellg() + chunk_data_size);
	}
}

void RegionData::tryLoad() const {
	ensure(dataRequest);
	if (!loaded){
		loadData(dataRequest(id));
		loaded= true;
	}
}

} // game
} // clover