#ifndef CLOVER_AUDIO_AUDIOSOURCEHANDLE_HPP
#define CLOVER_AUDIO_AUDIOSOURCEHANDLE_HPP

#include "build.hpp"
#include "audiosource.hpp"
#include "global/eventreceiver.hpp"
#include "util/countedpointer.hpp"

namespace clover {
namespace audio {

class Sound;
class SoundInstanceHandle;

class AudioSourceHandle : public global::EventReceiver {
public:
	AudioSourceHandle(AudioSource* audio_source= nullptr);

	virtual void onEvent(global::Event& e);

	void assignNewSource(AudioSource::Type type);

	void setPosition(const util::Vec2d& pos);
	void setVolume(real32 vol);

	SoundInstanceHandle playSound(const Sound& s);
	SoundInstanceHandle playSound(const util::Str8& soundname);
	
	explicit operator bool() const { return audioSource.get(); }

private:
	util::CountedPointer<AudioSource> audioSource;
};
	
} // audio
} // clover

#endif // CLOVER_AUDIO_AUDIOSOURCEHANDLE_HPP