#ifndef CLOVER_GAME_WORLDENTITY_MGR_HPP
#define CLOVER_GAME_WORLDENTITY_MGR_HPP

#include "build.hpp"
#include "global/eventreceiver.hpp"
#include "util/map.hpp"
#include "util/time.hpp"
#include "worldentity.hpp"

namespace clover {
namespace game {

class WeHandle;

/// game::WorldEntity manager
class WeMgr : public global::EventReceiver {
public:
	WeMgr();
	virtual ~WeMgr();

	void update();
	void spawnNewEntities();
	SizeType getEntityCount() const;

	util::DynArray<WorldEntity*> getGlobalEntities() const;

	/// Destroyes flagged entities
	void removeFlagged();

	/// Removes entities and chunks
	void removeAll();

	void onEvent(global::Event& e);
	
	/// Create global entity
	game::WeHandle createEntity(const util::Str8& type_name);
	/// Creates entity to a chunk
	game::WeHandle createEntity(const util::Str8& type_name, util::Vec2d pos, game::WorldChunk* c= nullptr);
	game::WeHandle minimalCreateEntity(game::WorldEntityId id= 0);
	
	void corruptionCheck() const;
	
private:
	// Perfoms shallow update for entities and takes care of chunk changes
	void shallowUpdate(const util::DynArray<game::WorldEntity*> update_needing);
};

} // util
} // clover

#endif // CLOVER_GAME_WORLDENTITY_MGR_HPP
