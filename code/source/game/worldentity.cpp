#include "debug/debugdraw.hpp"
#include "global/event.hpp"
#include "nodes/compositionnodelogicgroup.hpp"
#include "nodes/native_instances/nodeinstance_we_interface.hpp"
#include "nodes/nodetype.hpp"
#include "physics/object.hpp"
#include "resources/cache.hpp"
#include "util/profiling.hpp"
#include "worldentity_mgr.hpp"
#include "worldentity.hpp"

namespace clover {
namespace game {

template <typename WorldEntity>
util::Map<game::WorldEntityId, WorldEntity*> BaseWorldEntity<WorldEntity>::weMap;

template <typename WorldEntity>
BaseWorldEntity<WorldEntity>::BaseWorldEntity():
			active(true),
			global(false),
			spawningAllowed(false),
			spawned(false),
			remove(false),
			inChunk(nullptr),
			uniqueId(gWETable.assignUniqueId()), /// @todo This probably shouldn't be here (loaded entities will increment)
			tableIndex(gWETable.findFreeIndex()){

	gWETable[tableIndex]= thisAsWe();

	weMapIt= weMap.insert(std::make_pair(uniqueId, thisAsWe())).first;
}

template <typename WorldEntity>
BaseWorldEntity<WorldEntity>::~BaseWorldEntity(){
	gWETable[tableIndex]= 0;

	weMap.erase(weMapIt);
}

template <typename WorldEntity>
game::WeHandle BaseWorldEntity<WorldEntity>::assignId(game::WorldEntityId id){
	uniqueId= id;
	weMap.erase(weMapIt);
	weMapIt= weMap.insert(std::make_pair(uniqueId, thisAsWe())).first;
	if (id > gWETable.getNextUniqueId()) gWETable.setNextUniqueId(id + 1);
	return game::WeHandle(thisAsWe());
}

template <typename WorldEntity>
void BaseWorldEntity<WorldEntity>::saveFieldSerialize(WEPack& p) const {
	p.reset(uniqueId);
	saveFields >> p.getVarPack();
}

template <typename WorldEntity>
void BaseWorldEntity<WorldEntity>::saveFieldDeserialize(WEPack& p){
	PROFILE();
	ensure(p.getId() == uniqueId);

	p.prepareVarBlockReading();
	saveFields << p.getVarPack();
}

template <typename WorldEntity>
void BaseWorldEntity<WorldEntity>::setActive(bool active_){
	active= active_;
}

template <typename WorldEntity>
void BaseWorldEntity<WorldEntity>::spawn(){
	if (spawned)
		throw global::Exception("BaseWorldEntity::spawn(): already spawned");

	ensure(spawningAllowed);
	ensure(active);

	spawned= true;

	ensure(inChunk || isGlobal());
}

template <typename WorldEntity>
void BaseWorldEntity<WorldEntity>::setRemoveFlag(bool rm){
	remove= rm;
}

template <typename WorldEntity>
bool BaseWorldEntity<WorldEntity>::hasSpawned(){
	return spawned;
}

template class BaseWorldEntity<WorldEntity>;

//
// WorldEntity
//

WorldEntity::WorldEntity()
		: type(nullptr)
		, weInterface(nullptr)
		, errorState(false){
	DEF_WE_SAVE_FIELD(position)
	DEF_WE_SET_FUNC(position, setPosition)
}

void WorldEntity::weInit(){
	PROFILE();
	ensure(type);
	// Make sure that WeInterface is sorted to the beginning so that it's updated first
	if (!type->getNodeGroup().getNodes().empty())
		ensure_msg(type->getNodeGroup().getNodes().front()->getType().getName() == "WeInterface",
			"First node in node group was %s, %i",	type->getNodeGroup().getNodes().front()->getType().getName().cStr(),
													type->getNodeGroup().getNodes().front()->isUpdateRouteStart());
	weInterface= static_cast<nodes::WeInterfaceNodeInstance*>(&instance->getNode("WeInterface"));
	weInterface->setEntity(*this);
	weInterface->setOnDestroyCallback([this] (){
		savedAttributes= weInterface->getAttributeInfos();
	});
	if (!weInterface->isGlobal())
		weInterface->setPosition(getPosition());

	setGlobal(weInterface->isGlobal());
}

void WorldEntity::saveFieldSerialize(WEPack& p) const {
	PROFILE();
	util::Str8 type_name, serialized_attribs;

	
	if (type)
		type_name= type->getName();
	
	if (weInterface){
		PROFILE_("boost");
		std::stringstream ss;
		boost::archive::text_oarchive oa(ss);
		
		const auto& attribs= weInterface->getAttributeInfos();
		oa << attribs;
		
		serialized_attribs= util::Str8(ss.str());
	}
	
	Base::saveFieldSerialize(p);
	p.getVarPack() << type_name;
	p.getVarPack() << serialized_attribs;
}

void WorldEntity::saveFieldDeserialize(WEPack& p){
	PROFILE();
	Base::saveFieldDeserialize(p);
	
	util::Str8 type_name, serialized_attribs;
	p.getVarPack() >> type_name;
	p.getVarPack() >> serialized_attribs;
	
	if (!type_name.empty()){
		PROFILE();
		setType(type_name);
	}
	
	if (!serialized_attribs.empty()){
		nodes::WeInterfaceNodeInstance::AttributeInfos attribs;
		{ PROFILE();
			std::stringstream ss(serialized_attribs.cStr());
			boost::archive::text_iarchive ia(ss);
			ia >> attribs;
		}
		
		if (weInterface){
			PROFILE();
			weInterface->setAttributes(attribs);
		}
	}
	
}

void WorldEntity::setType(const util::Str8& name){
	PROFILE();
	type= &resources::gCache->getResource<game::WeType>(name);
	
	// Reset type if it has changed
	typeChangeListener.clear();
	typeChangeListener.listen(*type, [&] {
		//print(debug::Ch::General, debug::Vb::Trivial, "typechange detected in we %s", type->getName().cStr());
		instanceChangeListener.clear();
		instance= nullptr;
		setType(type->getName());
	});
	
	instanceChangeListener.clear();
	
	// In case of invalid type
	if (type->getResourceState() != resources::Resource::State::Loaded){
		errorState= true;
		instance= nullptr;
		return;
	}
	else
		errorState= false;

	instance= type->getNodeGroup().instantiate();

	instanceChangeListener.listen(*instance, [&] {
		print(debug::Ch::General, debug::Vb::Trivial, "Instance change detected in We");
		weInit();
		
		// Restore attributes
		if (weInterface)
			weInterface->setAttributes(savedAttributes);
		savedAttributes.clear();
		
		if (type->getResourceState() != resources::Resource::State::Loaded)
			errorState= true;
		else
			errorState= false;
	});
	
	weInit();
}

util::Str8 WorldEntity::getTypeName() const {
	if (type)
		return type->getName();
	else
		return util::Str8();
}

void WorldEntity::update(){
	if (instance)
		instance->update();
	
	shallowUpdate();

	if (errorState){
		debug::gDebugDraw->addText(
				util::Coord::W(position),
				util::Str8::format("Broken We: %s", type->getName().cStr()),
				util::Vec2d{0.5, 0.5},
				util::Color{1.0, 0.0, 0.0, 1.0});
	}
}

void WorldEntity::shallowUpdate(){
	if (weInterface){
		if (!weInterface->isGlobal())
			position= weInterface->getPosition();
		ensure( (getInChunk() == nullptr) == isGlobal() );
		if (weInterface->isRemoved())
			setRemoveFlag();
	}
}

nodes::UpdateLine WorldEntity::getUpdateLine() const {
	if (instance)
		return instance->getUpdateLine();
	else
		return nodes::UpdateLine();
}

void WorldEntity::onEvent(global::Event& e){
}

void WorldEntity::onEvent(const nodes::NodeEvent& e){
	if (weInterface)
		weInterface->receiveEvent(e);
}

void WorldEntity::setPosition(util::Vec2d pos){
	if (weInterface)
		weInterface->setPosition(position);
	position= pos;
}

bool WorldEntity::hasAttribute(const util::Str8& name) const {
	if (weInterface){
		return weInterface->hasAttribute(name);
	}
	return false;
}

void WorldEntity::setAttribute(const util::Str8& name, const boost::any& value){
	if (weInterface){
		weInterface->setAttribute(name, value);
	}
}

void WorldEntity::setActive(bool b){
	Base::setActive(b);
	if (weInterface)
		weInterface->setActive(b);
}

} // game
} // clover
