#ifndef CLOVER_AUDIO_AUDIO_MGR_HPP
#define CLOVER_AUDIO_AUDIO_MGR_HPP

#include "build.hpp"
#include "soundinstance.hpp"
#include "audiosource.hpp"
#include "audioreceiver.hpp"
#include "outputstream.hpp"
#include "util/time.hpp"
#include "util/linkedlist.hpp"

/// @todo Replace with util::Thread
#include <thread>

namespace clover {
namespace audio {

class Sound;
class AudioSourceHandle;
class AudioReceiverHandle;

/// Manages audio objects and feed thread
class AudioMgr {
public:
	AudioMgr();
	virtual ~AudioMgr();
	
	SizeType getSoundInstanceCount() const;
	SizeType getAudioSourceCount() const;
	SizeType getFreeChannelCount() const;

	SoundInstanceHandle playSound(const Sound& sound, AudioSource& owner_source);
	AudioSourceHandle createAudioSource(AudioSource::Type type);
	AudioReceiverHandle createAudioReceiver();
	
	void update();

private:
	util::LinkedList<SoundInstance> soundInstances;
	util::LinkedList<AudioSource> audioSources;
	util::LinkedList<AudioReceiver> audioReceivers;

	/// Actual playing started here
	void assignToChannel(util::LinkedList<SoundInstance>::Iter it);

	/// Samples from soundInstances to OutputStream
	std::thread feedThread;
	bool quitFeedThread;
	OutputStream& outputStream;
	void feedLoop();
};

} // audio
} // clover

#endif // CLOVER_AUDIO_AUDIO_MGR_HPP
