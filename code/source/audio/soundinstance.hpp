#ifndef CLOVER_AUDIO_SOUNDINSTANCE_HPP
#define CLOVER_AUDIO_SOUNDINSTANCE_HPP

#include "build.hpp"
#include "util/referencecountable.hpp"
#include "audiostream.hpp"
#include "sound.hpp"

#include <memory>

namespace clover {
namespace audio {

class AudioSource;
class SoundInstanceHandle;

/// Internal class of the audio system
/// Don't create directly, use SoundInstanceHandle
/// Created when sound starts playing and destroyed when it ends
class SoundInstance : public util::ReferenceCountable {
public:
	SoundInstance(const Sound& sound, AudioSource& owner_source);
	virtual ~SoundInstance();

	void create();
	void destroy();

	void assignToChannel(){ assignedToChannel= true; }
	bool isAssignedToChannel() const { return assignedToChannel; }
	
	typedef std::function<void (const SoundInstanceHandle& h)> OnEndCallbackType;
	void setOnEndCallback(const OnEndCallbackType& cb);
	const OnEndCallbackType& getOnEndCallback() const { return onEndCallback; }
	
	// Called asynchronously
	const Sound& getSound() const { ensure(sound); return *sound; }
	Sound::ChannelData getNextSamples(SizeType request_count, SizeType channel_id);
	bool eos(SizeType channel_id) const;
	void setVolume(const real32 vol){ volume= vol; }
	real32 getVolume() const { return volume; }
	AudioSource& getSource() const { ensure(source); return *source; }

private:
	bool created;
	bool assignedToChannel;
	bool finished;

	const Sound* sound;
	AudioSource* source;
	
	util::DynArray<std::weak_ptr<AudioStream>> streams;
	
	OnEndCallbackType onEndCallback;
	real32 volume; // Volume-multiplier
	
};

} // audio
} // clover

#endif // CLOVER_AUDIO_SOUNDINSTANCE_HPP