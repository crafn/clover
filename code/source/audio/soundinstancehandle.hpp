#ifndef CLOVER_AUDIO_SOUNDINSTANCEHANDLE_HPP
#define CLOVER_AUDIO_SOUNDINSTANCEHANDLE_HPP

#include "build.hpp"
#include "soundinstance.hpp"
#include "global/eventreceiver.hpp"
#include "util/countedpointer.hpp"

namespace clover {
namespace audio {

class AudioSourceHandle;

class SoundInstanceHandle : public global::EventReceiver {
public:
	SoundInstanceHandle(SoundInstance* sound_instance= nullptr);
	virtual ~SoundInstanceHandle();
	
	virtual void onEvent(global::Event& e);
	
	void setVolume(real32 mul);
	void setOnEndCallback(const SoundInstance::OnEndCallbackType& cb);
	const Sound& getSound() const;
	AudioSourceHandle getSource() const;
	
	explicit operator bool() const { return soundInstance.get(); }
	
private:
	util::CountedPointer<SoundInstance> soundInstance;
};

} // audio
} // clover

#endif // CLOVER_AUDIO_SOUNDINSTANCEHANDLE_HPP