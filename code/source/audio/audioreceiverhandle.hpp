#ifndef CLOVER_AUDIO_AUDIORECEIVERHANDLE_HPP
#define CLOVER_AUDIO_AUDIORECEIVERHANDLE_HPP

#include "build.hpp"
#include "audioreceiver.hpp"
#include "global/eventreceiver.hpp"
#include "util/countedpointer.hpp"

namespace clover {
namespace audio {

class AudioReceiverHandle : public global::EventReceiver {
public:
	AudioReceiverHandle(AudioReceiver* recv= nullptr);
	virtual ~AudioReceiverHandle();
	
	virtual void onEvent(global::Event& e);
	void assignNewReceiver();
	explicit operator bool() const { return audioReceiver.get(); }
	void setPosition(const util::Vec2d& pos);

private:
	util::CountedPointer<AudioReceiver> audioReceiver;
};

} // audio
} // clover

#endif // CLOVER_AUDIO_AUDIORECEIVERHANDLE_HPP