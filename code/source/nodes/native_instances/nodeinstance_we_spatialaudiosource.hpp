#ifndef CLOVER_NODES_NODEINSTANCE_WESPATIALAUDIOSOURCE_HPP
#define CLOVER_NODES_NODEINSTANCE_WESPATIALAUDIOSOURCE_HPP

#include "../nodeinstance.hpp"
#include "audio/audiosourcehandle.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class WeSpatialAudioSourceNodeInstance : public NodeInstance {
public:
	static CompNode* compNode();

	virtual ~WeSpatialAudioSourceNodeInstance(){}
	
	virtual void create() override;
	
private:

	InputSlot<SignalType::String>* soundNameIn;
	InputSlot<SignalType::Vec2>* positionIn;
	InputSlot<SignalType::Real>* volumeIn;
	InputSlot<SignalType::Trigger>* playIn;
	
	audio::AudioSourceHandle sourceHandle;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::WeSpatialAudioSourceNodeInstance> {
	static util::Str8 type(){ return "::WeSpatialAudioSourceNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_WESPATIALAUDIOSOURCE_HPP
