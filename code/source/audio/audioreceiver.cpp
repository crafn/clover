#include "audioreceiver.hpp"
#include "global/event.hpp"

namespace clover {
namespace audio {

AudioReceiver::AudioReceiver()
		: created(false)
		, lowpass(1.0){
}

AudioReceiver::AudioReceiver(AudioReceiver&& other)
		: created(other.created)
		, position(std::move(other.position))
		, lowpass(other.lowpass){

}

AudioReceiver::~AudioReceiver(){
	ensure(!created);
}

void AudioReceiver::create(){
	ensure(!created);
	
	global::Event e(global::Event::OnAudioReceiverCreate);
	e(global::Event::Object)= this;
	e.send();
	
	created= true;
}

void AudioReceiver::destroy(){
	ensure(created);
	
	global::Event e(global::Event::OnAudioReceiverDestroy);
	e(global::Event::Object)= this;
	e.send();
	
	created= false;
}

void AudioReceiver::setPosition(const util::Vec2d& pos){
	boost::mutex::scoped_lock lock(accessMutex);
	position= pos;
}

util::Vec2d AudioReceiver::getPosition() const {
	boost::mutex::scoped_lock lock(accessMutex);
	return position;
}

} // audio
} // clover