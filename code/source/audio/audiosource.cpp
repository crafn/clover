#include "audiosource.hpp"
#include "audio_mgr.hpp"
#include "audiosourcehandle.hpp"
#include "debug/debugdraw.hpp"
#include "global/event.hpp"
#include "util/mutex.hpp"

namespace clover {
namespace audio {

AudioSource::AudioSource(Type type_)
		: created(false)
		, type(type_)
		, volume(1.0){
}

AudioSource::AudioSource(AudioSource&& other)
		: created(other.created)
		, type(other.type)
		, position(std::move(other.position))
		, volume(other.volume)
		, soundInstances(std::move(other.soundInstances)){
}
	
AudioSource::~AudioSource(){
	ensure(!created);
}

void AudioSource::create(){
	ensure(!created);
	
	created= true;
	
	global::Event e(global::Event::OnAudioSourceCreate);
	e(global::Event::Object)= this;
	e.send();
	
	listenForEvent(global::Event::OnSoundInstanceDestroy);
}

void AudioSource::destroy(){
	ensure(created);
	
	created= false;
	
	global::Event e(global::Event::OnAudioSourceDestroy);
	e(global::Event::Object)= this;
	e.send();
}

void AudioSource::onEvent(global::Event& e){
	switch(e.getType()){
		case global::Event::OnSoundInstanceDestroy:
		
			for (auto it= soundInstances.begin(); it != soundInstances.end(); ++it){
				if (e(global::Event::Object).getPtr<SoundInstance>() == *it){
					soundInstances.erase(it);
					break;
				}
			}
			
		break;
		
		default: break;
	}
}

SoundInstanceHandle AudioSource::playSound(const Sound& s){
	return gAudioMgr->playSound(s, *this);
}

void AudioSource::addSoundInstance(SoundInstance& h){
	soundInstances.pushBack(&h);
}

void AudioSource::setPosition(const util::Vec2d& pos){
	util::LockGuard<util::Mutex> lock(accessMutex); 
	ensure(type == Type::Spatial);
	position= pos;
}

util::Vec2d AudioSource::getPosition() const {
	util::LockGuard<util::Mutex> lock(accessMutex); 
	return position; 
}

void AudioSource::update(){
	util::LockGuard<util::Mutex> lock(accessMutex); 
	
	debug::gDebugDraw->addFilledCircle(position, 0.3,  {0.9, 0.5, 0.3, 0.5});
	debug::gDebugDraw->addText(position, "AudioSource", util::Vec2d{0.5f,0.5f});

	//if (!entities.empty()){
		debug::gDebugDraw->addCircle(position, 0.4, {0.9f,0.3f,0.2f, 0.3f});
	//}

}

bool AudioSource::hasFinished() const {
	return soundInstances.empty();
}

} // audio
} // clover