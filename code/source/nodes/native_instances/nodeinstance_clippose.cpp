#include "nodeinstance_clippose.hpp"
#include "animation/clip.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace nodes {

void ClipPoseNodeInstance::create(){
	clip= nullptr;
	
	clipIn= addInputSlot<SignalType::String>("clip");
	interpolateIn= addInputSlot<SignalType::Boolean>("interpolate");
	phaseIn= addInputSlot<SignalType::Real>("phase");
	
	poseOut= addOutputSlot<SignalType::ArmaturePose>("pose");
	
	clipIn->setValueReceived();
	clipIn->setOnReceiveCallback([&] (){
		clip= &resources::gCache->getResource<animation::Clip>(clipIn->get());
		setUpdateNeeded();
	});
	
	interpolateIn->setOnReceiveCallback([&] (){
		setUpdateNeeded();
	});
	
	phaseIn->setOnReceiveCallback([&] (){
		setUpdateNeeded();
	});
	
	setUpdateNeeded(false);
}

void ClipPoseNodeInstance::update(){
	ensure(clip);
	// Send poses only if clip is loaded
	if (clip->getResourceState() == resources::Resource::State::Loaded)
		poseOut->send(clip->getPose(phaseIn->get(), interpolateIn->get()));
	setUpdateNeeded(false);
}

} // nodes
} // clover