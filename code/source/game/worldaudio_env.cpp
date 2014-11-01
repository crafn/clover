#include "worldaudio_env.hpp"
#include "resources/cache.hpp"
#include "audio/audiosourcehandle.hpp"
#include "audio/soundinstancehandle.hpp"

namespace clover {
namespace game {

WorldAudioEnv::WorldAudioEnv(){
}

WorldAudioEnv::~WorldAudioEnv(){
}

void WorldAudioEnv::setAmbientTrack(const util::Str8& resource_name){
	audio::AudioSourceHandle source;
	source.assignNewSource(audio::AudioSource::Type::Global);
	auto sound_instance= source.playSound(resource_name);
	
	// Loop
	if (sound_instance)
		sound_instance.setOnEndCallback(std::bind(&WorldAudioEnv::onAmbientEnd, this, std::placeholders::_1));
}

void WorldAudioEnv::onAmbientEnd(const audio::SoundInstanceHandle& h) const {
	if (h && h.getSource()){
		auto sound_instance= h.getSource().playSound(h.getSound());
		if (sound_instance){
			sound_instance.setOnEndCallback(std::bind(&WorldAudioEnv::onAmbientEnd, this, std::placeholders::_1));
		}
	}
}

} // game
} // clover