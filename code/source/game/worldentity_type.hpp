#ifndef CLOVER_GAME_WORLDENTITY_TYPE_HPP
#define CLOVER_GAME_WORLDENTITY_TYPE_HPP

#include "build.hpp"
#include "nodes/nodeinstancegroup.hpp"
#include "nodes/native_instances/nodeinstance_we_interface.hpp"
#include "resources/resource.hpp"

namespace clover {
namespace game {

class WeType;

} // game
namespace resources {

template <>
struct ResourceTraits<game::WeType> {
	DECLARE_RESOURCE_TRAITS(game::WeType, String)

	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"),
							AttributeDef::Resource("nodeGroup"));

	typedef SubCache<game::WeType> SubCacheType;

	static util::Str8 typeName(){ return "WeType"; }
	static util::Str8 identifierKey(){ return "name"; }

	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static bool expired(const game::WeType& type){ return false; }
};

} // resources
namespace nodes {

class CompositionNodeLogicGroup;
class WeInterfaceNodeInstance;

} // nodes
namespace game {

class WeType : public resources::Resource {
public:
	DECLARE_RESOURCE(WeType)
	
	WeType();
	virtual ~WeType(){}
	
	virtual void resourceUpdate(bool load, bool force=false);
	virtual void createErrorResource();
	
	const util::Str8& getName() const { return nameAttribute.get(); }
	const nodes::CompositionNodeLogicGroup& getNodeGroup() const;
	
private:
	RESOURCE_ATTRIBUTE(String, nameAttribute)
	RESOURCE_ATTRIBUTE(Resource, nodeGroupAttribute)
	
	const nodes::CompositionNodeLogicGroup* nodeGroup;
	
	util::CbListener<util::OnChangeCb> nodeGroupChangeListener;
};

} // game
} // clover

#endif // CLOVER_GAME_WORLDENTITY_TYPE_HPP