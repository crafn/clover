#include "nodeinstance_we_spatialaudiosource.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* WeSpatialAudioSourceNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->addInputSlot("soundName", SignalType::String, util::Str8(""));
	n->addInputSlot("position", SignalType::Vec2);
	n->addInputSlot("volume", SignalType::Real, 1.0);
	n->addInputSlot("play", SignalType::Trigger);
	return n;
}

void WeSpatialAudioSourceNodeInstance::create(){
	soundNameIn= addInputSlot<SignalType::String>("soundName");
	positionIn= addInputSlot<SignalType::Vec2>("position");
	volumeIn= addInputSlot<SignalType::Real>("volume");
	playIn= addInputSlot<SignalType::Trigger>("play");
	
	sourceHandle.assignNewSource(audio::AudioSource::Type::Spatial);
	
	positionIn->setOnReceiveCallback(+[] (WeSpatialAudioSourceNodeInstance* self){
		self->sourceHandle.setPosition(self->positionIn->get());
	});
	
	volumeIn->setOnReceiveCallback(+[] (WeSpatialAudioSourceNodeInstance* self){
		self->sourceHandle.setVolume(self->volumeIn->get());
	});
	
	playIn->setOnReceiveCallback(+[] (WeSpatialAudioSourceNodeInstance* self){
		self->sourceHandle.playSound(self->soundNameIn->get());
	});
}

} // nodes
} // clover
