#ifndef CLOVER_GAME_WORLDENTITY_H
#define CLOVER_GAME_WORLDENTITY_H

#include "build.hpp"
#include "nodes/nodeevent.hpp"
#include "nodes/updateline.hpp"
#include "util/fh_save.hpp"
#include "util/math.hpp"
#include "util/map.hpp"
#include "util/unique_ptr.hpp"
#include "visual/model.hpp"
#include "worldentity_pack.hpp"
#include "worldentity_macros.hpp"
#include "worldentity_table.hpp"
#include "worldentity_type.hpp"

namespace clover {
namespace global {

class Event;

} // global
namespace game {

class WorldEntity;
class WeHandle;
class WeMgr;
class WorldChunk;
class WEPack;

template <typename WorldEntity>
class BaseWorldEntity {
public:
	static const util::Map<game::WorldEntityId, WorldEntity*>& getWorldEntityMap(){ return weMap; }

	BaseWorldEntity();
	BaseWorldEntity(const BaseWorldEntity&)= delete;
	BaseWorldEntity& operator=(const BaseWorldEntity&)= delete;
	virtual ~BaseWorldEntity();

	game::WorldEntityId getId() const { return uniqueId; }

	/// Only WEMgr is allowed to call this
	/// @note Breaks all handles pointing to this entity
	game::WeHandle assignId(game::WorldEntityId id);
	
	void saveFieldSerialize(WEPack& p) const; // Save
	void saveFieldDeserialize(WEPack& p); // Load

	void spawn();
	bool isSpawned() const { return spawned; }

	void setRemoveFlag(bool rm=true);
	bool getRemoveFlag() const { return remove; }

	bool hasSpawned();

	void setActive(bool active);
	/// When entity isn't active, nobody except the one who disabled should be
	/// modifying its state. (e.g. deserialization can be threaded)
	bool isActive() const { return active; }
	
	game::WorldChunk* getInChunk(){ return inChunk; }
	void setInChunk(game::WorldChunk* c){ inChunk= c; }

	bool isGlobal() const { return global; }

	static void updateAll(real32);

	void allowSpawning(bool s=true){
		ensure(!spawned);
		spawningAllowed= s;
	}
	
	bool isSpawningAllowed(){
		ensure(!spawned);
		return spawningAllowed;
	}
	
private:

	WorldEntity* thisAsWe(){ return static_cast<WorldEntity*>(this); }
	const WorldEntity* thisAsWe() const { return static_cast<const WorldEntity*>(this); }
	
	/// Entity exists and interacts in game world
	/// False when chunk is inactive
	bool active;
	bool global;

	bool spawningAllowed;
	bool spawned;

	/// If true, entity is destroyed at the end of frame
	bool remove;

	/// Manager sets
	/// Null for global entities
	game::WorldChunk* inChunk;
	
protected:
	virtual void pureVirtual()= 0; // Make this class abstract
	util::SaveFieldHolder saveFields;
	
	void setGlobal(bool b){ global= b; }

private:
	friend class WeHandle;
	friend class game::WeMgr;

	game::WorldEntityId uniqueId;

	int32 tableIndex;

	// Second structure for worldEntities, fast search by unique id
	static util::Map<game::WorldEntityId, WorldEntity*> weMap;
	typename util::Map<game::WorldEntityId, WorldEntity*>::iterator weMapIt;
};


/// Implements the actual game logic of world entities
/// WorldEntity and BaseWorldEntity were kept separate because it's probably cleaner that way
class WorldEntity : public BaseWorldEntity<WorldEntity> {
public:
	using Base= BaseWorldEntity;
	using ThisClass= WorldEntity;
	
	WorldEntity();
	virtual ~WorldEntity(){}
	
	void setType(const util::Str8& name);
	const WeType& getType() const { return *NONULL(type); }
	util::Str8 getTypeName() const;
	
	void update();
	/// Use this if nodes are updated with UpdateLine
	void shallowUpdate();

	nodes::UpdateLine getUpdateLine() const;
	bool isUpdateNeeded() const { return isActive() && instance && !instance->isUpdateNoop(); }
	
	void onEvent(global::Event& e);
	void onEvent(const nodes::NodeEvent& e);
	
	void setPosition(util::Vec2d);
	
	util::Vec2d getPosition() const { return position; }

	bool hasAttribute(const util::Str8& name) const;
	void setAttribute(const util::Str8& name, const boost::any& value);
	template <typename T>
	void setAttribute(const util::Str8& name, const T& value){ setAttribute(name, boost::any(value)); }
	
	void saveFieldSerialize(WEPack& p) const;
	void saveFieldDeserialize(WEPack& p);
	
	void setActive(bool b= true);
	
protected:
	virtual void pureVirtual() override {}
	void weInit();
	
	const game::WeType* type;
	
	util::Vec2d position;
	
	/// In case of node group recreation attributes must remain
	nodes::WeInterfaceNodeInstance::AttributeInfos savedAttributes;
	
	util::UniquePtr<nodes::NodeInstanceGroup> instance;
	
	nodes::WeInterfaceNodeInstance* weInterface;
	
	// These two might sometimes be triggered on the same frame, but that shouldn't matter too much
	// A better (but not simpler) way would be to have different callbacks for different types of change
	// If slows too much, use events
	util::CbListener<util::OnChangeCb> instanceChangeListener; // When instance is changed
	util::CbListener<util::OnChangeCb> typeChangeListener; // When type changes (e.g. when changing nodeGroup of game::WeType)
	
	bool errorState;
};

} // game
namespace util {

template <>
struct TypeStringTraits<game::BaseWorldEntity<game::WorldEntity>> {
	static util::Str8 type(){ return "WorldEntity"; }
};

template <>
struct TypeStringTraits<game::WorldEntity> {
	static util::Str8 type(){ return "WorldEntity"; }
};

} // util
} // clover

#endif // CLOVER_GAME_WORLDENTITY_H
