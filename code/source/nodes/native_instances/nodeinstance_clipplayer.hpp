#ifndef CLOVER_NODES_NODEINSTANCE_CLIPPLAYER_HPP
#define CLOVER_NODES_NODEINSTANCE_CLIPPLAYER_HPP

#include "build.hpp"
#include "../nodeinstance.hpp"

namespace clover {
namespace animation {

class Clip;

} // animation
namespace nodes {

class ClipPlayerNodeInstance : public NodeInstance {
public:
	
	virtual ~ClipPlayerNodeInstance(){}
	
	virtual void create() override;
	virtual void update() override;
	
private:
	void stopPlaying();

	InputSlot<SignalType::String>* clipIn;
	InputSlot<SignalType::Real>* playIn;
	InputSlot<SignalType::Real>* timeScaleIn;
	InputSlot<SignalType::Integer>* loopCountIn;
	InputSlot<SignalType::Boolean>* interpolateIn;
	
	OutputSlot<SignalType::ArmaturePose>* poseOut;
	OutputSlot<SignalType::Real>* phaseOut;
	OutputSlot<SignalType::Trigger>* onEndOut;
	
	const animation::Clip* clip;
	real64 time;
	int32 currentLoop;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::ClipPlayerNodeInstance> {
	static util::Str8 type(){ return "::ClipPlayerNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_CLIPPLAYER_HPP
