#include "audiosourcehandle.hpp"
#include "audio_mgr.hpp"
#include "audiosource.hpp"
#include "sound.hpp"
#include "soundinstancehandle.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace audio {

AudioSourceHandle::AudioSourceHandle(AudioSource* audio_source):
	audioSource(audio_source){
		
	if (audioSource)
		listenForEvent(global::Event::OnAudioSourceDestroy);
}

void AudioSourceHandle::onEvent(global::Event& e){
	switch(e.getType()){
		
		case global::Event::OnAudioSourceDestroy:
			if (e(global::Event::Object).getPtr<AudioSource>() == audioSource.get()){
				audioSource= 0;
				unlistenForEvent(global::Event::OnAudioSourceDestroy);
			}
		break;
		
		default: break;
	}
}

void AudioSourceHandle::assignNewSource(AudioSource::Type type){
	if (!audioSource)
		listenForEvent(global::Event::OnAudioSourceDestroy);
	
	operator=(gAudioMgr->createAudioSource(type));
	
	ensure(audioSource);
}

void AudioSourceHandle::setPosition(const util::Vec2d& pos){
	ensure(audioSource);
	audioSource->setPosition(pos);
}

void AudioSourceHandle::setVolume(real32 vol){
	ensure(audioSource);
	audioSource->setVolume(vol);
}

SoundInstanceHandle AudioSourceHandle::playSound(const Sound& s){
	ensure_msg(audioSource, "Invalid AudioSourceHandle");
	return (audioSource->playSound(s));
}

SoundInstanceHandle AudioSourceHandle::playSound(const util::Str8& name){
	return (playSound(resources::gCache->getResource<Sound>(name)));
}

} // audio
} // clover