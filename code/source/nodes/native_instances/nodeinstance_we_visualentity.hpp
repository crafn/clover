#ifndef CLOVER_NODES_NODEINSTANCE_WE_VISUALENTITY_HPP
#define CLOVER_NODES_NODEINSTANCE_WE_VISUALENTITY_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"
#include "visual/entity.hpp"
#include "visual/entity_def_model.hpp"

namespace clover {
namespace nodes {

class WeVisualEntityNodeInstance : public NodeInstance {
public:
	virtual void create();
	virtual void update();
	
private:
	static void updateHighlight(	visual::Entity& entity,
									util::Color normal_c,
									real32 mul);
	util::Color getInputColorMul() const;

	InputSlot<SignalType::String>* entityInput;
	InputSlot<SignalType::Boolean>* activeInput;
	InputSlot<SignalType::SrtTransform3>* transformInput;
	InputSlot<SignalType::ArmaturePose>* poseInput;
	InputSlot<SignalType::Vec4>* colorMulInput;
	InputSlot<SignalType::EventArray>* eventsInput;
	
	visual::Entity entity;
	float highlightLerp= 0.0f;
};

}
namespace util {

template <>
struct TypeStringTraits<nodes::WeVisualEntityNodeInstance> {
	static util::Str8 type(){ return "::WeVisualEntityNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_WE_VISUALENTITY_HPP
