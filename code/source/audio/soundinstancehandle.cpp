#include "soundinstancehandle.hpp"
#include "audiosourcehandle.hpp"

namespace clover {
namespace audio {

SoundInstanceHandle::SoundInstanceHandle(SoundInstance* sound_instance)
		: soundInstance(sound_instance){
	listenForEvent(global::Event::OnSoundInstanceDestroy);
}

SoundInstanceHandle::~SoundInstanceHandle(){
}

void SoundInstanceHandle::onEvent(global::Event& e){
	switch (e.getType()){
		case global::Event::OnSoundInstanceDestroy:
			if (e(global::Event::Object).getPtr<SoundInstance>() == soundInstance.get()){
				soundInstance= 0;
			}
		break;
		
		default: ensure(0);
	}
}

void SoundInstanceHandle::setVolume(real32 mul){
	ensure(soundInstance);
	soundInstance->setVolume(mul);
}

void SoundInstanceHandle::setOnEndCallback(const SoundInstance::OnEndCallbackType& cb){
	ensure(soundInstance);
	soundInstance->setOnEndCallback(cb);
}

const Sound& SoundInstanceHandle::getSound() const {
	ensure(soundInstance);
	return soundInstance->getSound();
}

AudioSourceHandle SoundInstanceHandle::getSource() const {
	ensure(soundInstance);
	return AudioSourceHandle(&soundInstance->getSource());
}

} // audio
} // clover