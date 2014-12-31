#include "nodeinstance_we_spatialaudiosource.hpp"

namespace clover {
namespace nodes {

CompNode* WeSpatialAudioSourceNodeInstance::compNode()
{
	auto n= new CompNode{};
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
	
	positionIn->setOnReceiveCallback([this] (){
		sourceHandle.setPosition(positionIn->get());
	});
	
	volumeIn->setOnReceiveCallback([this] (){
		sourceHandle.setVolume(volumeIn->get());
	});
	
	playIn->setOnReceiveCallback([this] (){
		sourceHandle.playSound(soundNameIn->get());
	});
}

} // nodes
} // clover
