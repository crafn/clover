#include "nodeinstance_clipplayer.hpp"
#include "animation/clip.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* ClipPlayerNodeInstance::compNode()
{
	CompositionNodeLogic* n= new CompositionNodeLogic{};
	n->addInputSlot("clip", SignalType::String);
	n->addInputSlot("play", SignalType::Real, 0.0); // Value is starting phase
	n->addInputSlot("timeScale", SignalType::Real, 1.0);
	n->addInputSlot("loopCount", SignalType::Integer, (int64)-1);
	n->addInputSlot("interpolate", SignalType::Boolean, true);
	n->addOutputSlot("pose", SignalType::ArmaturePose);
	n->addOutputSlot("phase", SignalType::Real);
	n->addOutputSlot("onEnd", SignalType::Trigger);
	return n;
}

void ClipPlayerNodeInstance::create(){
	clip= nullptr;
	time= 0.0;
	currentLoop= 0;
	
	clipIn= addInputSlot<SignalType::String>("clip");
	playIn= addInputSlot<SignalType::Real>("play");
	timeScaleIn= addInputSlot<SignalType::Real>("timeScale");
	loopCountIn= addInputSlot<SignalType::Integer>("loopCount");
	interpolateIn= addInputSlot<SignalType::Boolean>("interpolate");
	
	poseOut= addOutputSlot<SignalType::ArmaturePose>("pose");
	phaseOut= addOutputSlot<SignalType::Real>("phase");
	onEndOut= addOutputSlot<SignalType::Trigger>("onEnd");
	
	clipIn->setValueReceived();
	clipIn->setOnReceiveCallback([&] (){
		clip= &global::g_env.resCache->getResource<animation::Clip>(clipIn->get());
	});
	
	playIn->setOnReceiveCallback([&] (){
		time= playIn->get()*clip->getTime(); // Value is starting phase
		currentLoop= 0;
		setUpdateNeeded();
	});
	
	setUpdateNeeded(false);
}

void ClipPlayerNodeInstance::update(){
	ensure(clip);
	
	// Send poses only if clip is loaded
	if (clip->getResourceState() != resources::Resource::State::Loaded)
		return;
	
	real32 time_scale= timeScaleIn->get();
	if (time_scale < 0.0){
		print(debug::Ch::Nodes, debug::Vb::Moderate, "ClipPlayerNodeInstance: negative timeScale not supported: %f", time_scale);
		time_scale= 0.0;
	}
	
	time += util::gGameClock->getDeltaTime()*time_scale;
	
	const real32 max_time= clip->getTime();
	while (time > max_time){
		onEndOut->send();

		if (loopCountIn->get() >= 0 && currentLoop >= loopCountIn->get()){
			time= max_time;
			stopPlaying();
			break;
		}
		
		++currentLoop;
		time -= max_time;
	}
	
	real32 phase= time/max_time;
	poseOut->send(clip->getPose(phase, interpolateIn->get()));
	phaseOut->send(phase);
}

void ClipPlayerNodeInstance::stopPlaying(){
	setUpdateNeeded(false);
}

} // nodes
} // clover
