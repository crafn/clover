#include "nodeinstance_we_spatialaudiosource.hpp"

namespace clover {
namespace nodes {

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