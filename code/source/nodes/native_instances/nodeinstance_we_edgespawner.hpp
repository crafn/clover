#ifndef CLOVER_NODES_NATIVE_INSTANCES_NODEINSTANCE_WE_EDGESPAWNER_HPP
#define CLOVER_NODES_NATIVE_INSTANCES_NODEINSTANCE_WE_EDGESPAWNER_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"
#include "util/arrayview.hpp"

namespace clover {
namespace game {

class WeType;

} // game
namespace nodes {

class WeEdgeSpawnerNodeInstance final : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual void create() override;
	virtual void update() override;

	const game::WeType* getEdgeType() const { return edgeType; }
	const game::WeType* getSpawnerType() const { return spawnerType; }

private:
	InputSlot<SignalType::String>* edgeEntityIn;
	InputSlot<SignalType::WeHandle>* weIn;
	InputSlot<SignalType::Trigger>* spawnIn;

	const game::WeType* edgeType= nullptr;
	const game::WeType* spawnerType= nullptr;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::WeEdgeSpawnerNodeInstance> {
	static util::Str8 type(){ return "::WeEdgeSpawnerNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NATIVE_INSTANCES_NODEINSTANCE_WE_EDGESPAWNER_HPP
