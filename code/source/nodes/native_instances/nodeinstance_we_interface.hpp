#ifndef CLOVER_NODES_NODEINSTANCE_WE_INTERFACE_HPP
#define CLOVER_NODES_NODEINSTANCE_WE_INTERFACE_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace game {

class WorldEntity;

} // game
namespace nodes {

class WeInterfaceNodeInstance : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual ~WeInterfaceNodeInstance();
	
	void setEntity(game::WorldEntity& we);

	virtual void create() override;
	virtual void update() override;
	
	void setActive(bool b= true);
	bool isActive() const { return activeInput->get(); }
	
	bool isGlobal() const { return transformInput == nullptr; } 
	
	void setPosition(util::Vec2d t);
	util::Vec2d getPosition();
	
	void receiveEvent(const NodeEvent& e);
	
	bool isRemoved() const { return removed; }
	
	bool hasAttribute(const util::Str8& name) const;

	/// Sets initial values of WeInterface node attributes
	/// Doesn't throw Exceptions, prints on error
	void setAttribute(const util::Str8& name, const util::Any& value);
	
	/// Used for serializing and deserializing
	struct AttributeInfo {
		uint32 nameHash;
		SignalType signalType;
		util::Any value;
		
		template <typename Archive>
		void serialize(Archive& ar, uint32 ver){
			ar & signalType;
			ar & nameHash;
			RuntimeSignalTypeTraits::serializeValue(signalType, ar, ver, value);
		}
	};
	
	using AttributeInfos= util::DynArray<AttributeInfo>;
	
	void setAttributes(const AttributeInfos& attribs);
	AttributeInfos getAttributeInfos() const;
	
	using OnDestroy= std::function<void ()>;
	void setOnDestroyCallback(OnDestroy cb){ onDestroy= cb; }
	
private:
	void setAttribute(uint32 name_hash, const util::Any& value, const util::Str8& disp_name= "");
	/// Forward values from attributeInputs to attributeOutputs
	void sendAttributes();
	
	InputSlot<SignalType::Boolean>* activeInput;
	InputSlot<SignalType::Trigger>* removeInput;
	BaseInputSlot* transformInput= nullptr;
	
	OutputSlot<SignalType::Boolean>* activeOutput;
	OutputSlot<SignalType::EventArray>* eventsOutput;
	OutputSlot<SignalType::WeHandle>* weOutput;
	BaseOutputSlot* transformOutput= nullptr;
	
	/// <Hash32<Str>, Slot>
	util::Map<uint32, BaseInputSlot*> attributeInputs;
	util::Map<uint32, BaseOutputSlot*> attributeOutputs;
	
	bool sendActive= false;
	bool sendTransform= false;
	bool firstUpdate;
	bool removed= false;
	OnDestroy onDestroy;
	util::DynArray<NodeEvent> events;
	game::WorldEntity* entity= nullptr;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::WeInterfaceNodeInstance> {
	static util::Str8 type(){ return "::WeInterfaceNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_WE_INTERFACE_HPP
