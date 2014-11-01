#ifndef CLOVER_GAME_WORLDENTITY_HANDLECONNECTION_HPP
#define CLOVER_GAME_WORLDENTITY_HANDLECONNECTION_HPP

#include "build.hpp"
#include "worldentity.hpp"
#include "worldentity_handle.hpp"

namespace clover {
namespace game {

/// Connects owner and pointee We of a single handle.
/// Needed for solving groups of dependent entities in different
/// chunks at save/load time
class StrictHandleConnection {
public:
	StrictHandleConnection(game::WorldEntity& owner, game::WeHandle& handle);
	StrictHandleConnection(const StrictHandleConnection&) = delete;
	StrictHandleConnection(StrictHandleConnection&&);
	
	virtual ~StrictHandleConnection();
	
	void resetHandle(game::WeHandle& h);
	
	game::WeHandle& getHandle(){ return *handle; }
	
	game::WorldEntity& getOwner(){ return *owner; }
	
	static util::PtrTable<StrictHandleConnection>::Iter begin(){
		return strictHandleTable.begin();
	}
	
	static util::PtrTable<StrictHandleConnection>::Iter end(){
		return strictHandleTable.end();
	}
	
	static uint32 countConnections();

private:
	static util::PtrTable<StrictHandleConnection> strictHandleTable;
	int32 tableIndex;

	game::WorldEntity* owner;
	game::WeHandle* handle;
};

} // game
} // clover

#endif // CLOVER_GAME_WORLDENTITY_HANDLECONNECTION_HPP