#ifndef CLOVER_GAME_WORLDCHUNK_HPP
#define CLOVER_GAME_WORLDCHUNK_HPP

#include "build.hpp"
#include "util/arrayview.hpp"
#include "util/math.hpp"
#include "util/dyn_array.hpp"
#include "util/linkedlist.hpp"
#include "util/map.hpp"
#include "util/ptrtable.hpp"
#include "util/set.hpp"
#include "worldentity.hpp"
#include "worldgrid.hpp"

namespace clover {
namespace game {

class WorldChunk {
public:
	using ChunkSet= util::Set<WorldChunk*>;
	using ClusterSet= util::Set<ChunkSet>;

	static util::DynArray<WorldChunk*> nearestSorted(ChunkVec target, ChunkSet chunks);
	static util::DynArray<ChunkSet> nearestSorted(ChunkVec target, ClusterSet clusters);

	static constexpr uint32 width= game::WorldGrid::chunkWidthInBlocks;

	enum Side {
		Right,
		Down,
		Left,
		Up
	};

	enum class State {
		None, // Default
		Active,
		Creating, // Loading or generating
		Serializing, // Serialization is performed (possibly in other thread)
		Destroying // Destroying after saved to file
	};

	WorldChunk(ChunkVec pos= util::Vec2i{0,0});
	virtual ~WorldChunk();

	void setState(State s);
	State getState() const { return state; }

	void setPosition(ChunkVec p);
	ChunkVec getPosition() const;

	void setSide(WorldChunk& chunk);
	void setSide(Side s, WorldChunk* c){ sides[s]= c; }

	WorldChunk* getSide(Side s);


	util::PtrTable<game::WorldEntity>& getEntityTable(){ return entities; }

	void addEntity(game::WorldEntity& obj);
	void removeEntity(game::WorldEntity& obj);

	SizeType getEntityCount() const;

	void clearDependencies();
	void addDependency(WorldChunk& c);
	void removeDependency(WorldChunk& c);

	uint32 getDependencyCount() const { return dependencyMap.size(); }
	util::Map<util::Vec2i, WorldChunk*>::iterator depBegin(){ return dependencyMap.begin(); }
	util::Map<util::Vec2i, WorldChunk*>::iterator depEnd(){ return dependencyMap.end(); }

	/// @return A group of dependent chunks in which 'this' belongs
	ChunkSet getCluster() const;

	/// Removes at maximum max_count entities
	/// @return Removed entity count
	SizeType updateDestroying(SizeType max_count);

	/// Must be in Serializing -state before calling
	util::DynArray<uint8> getEntityData() const;

	/// In savefile
	static constexpr uint16 chunkStartTag= 0xACDC;
	static constexpr uint16 entityStartTag= 0x1EE1;

	/// Serializes bunch of entities with chunk serialization
	static util::DynArray<uint8> serializeEntities(
			util::ArrayView<const WorldEntity* const> entities);

	/// Null chunk means entity is treated as global
	/// @param data_pack should contain stuff serialized with chunk serialization
	static void createNextEntity(util::RawPack& data_pack, WorldChunk* chunk);

protected:
	void removeEntity(SizeType index);

	State state;

	util::PtrTable<game::WorldEntity> entities;

	// Dependencies are rebuilt only when chunk is saved
	util::Map<util::Vec2i, WorldChunk*> dependencyMap;

	WorldChunk *sides[4];

	// Position in chunk coordinates
	util::Vec2i position;
};

} // game
} // clover

#endif // CLOVER_GAME_WORLDCHUNK_HPP
