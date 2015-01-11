#ifndef CLOVER_GAME_WORLDENTITY_HANDLE_HPP
#define CLOVER_GAME_WORLDENTITY_HANDLE_HPP

#include "build.hpp"
#include "worldentity_table.hpp"

namespace clover {
namespace game {

class WorldEntity;
class StrictHandleConnection;

/// Smart WorldEntity handle
class ENGINE_API WeHandle {
public:
	WeHandle(const game::WorldEntity* e=0);
	~WeHandle();
	
	WeHandle(const WeHandle& h);
	WeHandle(WeHandle&& h);
	
	WeHandle& operator=(const WeHandle& h);
	
	bool operator==(const WeHandle& h) const { return entityId == h.entityId; }

	void setId(game::WorldEntityId id);
	game::WorldEntityId getId() const { return entityId; }
	
	void reset();
	bool isAssigned();
	
	game::WorldEntity* get() const;
	game::WorldEntity& ref() const;

	/// @return Is usable
	explicit operator bool();
	explicit operator bool() const;

	game::WorldEntity* operator->() const;

	static void fixLostHandles(game::WorldEntity* w);
	static void fixLostHandles();
	
	static bool isNewLostHandles(){ return newLostHandle; }
	
	static uint32 countLostHandles();
	
	void setStrict(game::WorldEntity& owner);
	bool isStrict() const { return strictConnection; }
	
	template <typename Archive>
	void serialize(Archive& ar, const uint32 ver) {
		ensure_msg(!isStrict(), "Strict handle node-serialization not yet supported");
		if (Archive::is_saving::value) {
			game::WorldEntityId id= getId();
			ar & id;
		} else {
			game::WorldEntityId id;
			ar & id;
			setId(id);
		}
	}

private:
	game::WorldEntityId entityId;

	uint32 tableIndex;
	int32 lostIndex;

	static util::PtrTable<WeHandle> lostTable;

	/// True when there is at least one lost handle
	static bool newLostHandle;

	/// @return Handle can be used
	bool isGood() const;
	bool isLost() const;
	void updateLostness();
	
	/// For saving and loading
	StrictHandleConnection *strictConnection;
};

} // game
} // clover

#endif // CLOVER_GAME_WORLDENTITY_HANDLE_HPP
