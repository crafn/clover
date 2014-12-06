#ifndef CLOVER_AUDIO_OUTPUTCHANNEL_HPP
#define CLOVER_AUDIO_OUTPUTCHANNEL_HPP

#include "build.hpp"
#include "util/atomic.hpp"
#include "util/ringbuffer.hpp"
#include "smoothed.hpp"

namespace clover {
namespace audio {

class SoundInstance;
class AudioSource;
class AudioReceiver;

static constexpr SizeType soundChannelIdNone= (SizeType)-1;

/// Holds information of one playing sound, shared between audio managing and audio output
struct OutputChannel {
	
	enum class State {
		Free,  /// No audio is assigned for this channel. Audio manager controls channel state
		Start, /// Feeding thread is doing initializations. Feed thread controls channel state
		Play,  /// Feeding thread is pushing samples and device thread reading. Device thread controls channel state
		Close  /// Feeding thread deinitializes. Feed thread controls channel state
	};
	
	OutputChannel();
	
	/// Read by everyone, written by whoever is in control, @see State
	util::Atomic<State> state;
	
	///
	/// AudioMgr writes and reads (maybe these should be in another place then)
	///
	
	util::Atomic<SoundInstance*> soundInstance;
	util::Atomic<AudioSource*> audioSource;
	util::Atomic<AudioReceiver*> audioReceiver;
	
	/// One channel of the sound
	struct AudioChannel {
		
		struct Sample {
			// L, R
			real32 amplitude; // [0,1]
			real32 pan; // [-1, 1]
		};
		
		AudioChannel();
	
		///
		/// AudioMgr writes, Device thread reads
		///
		
		/// Audio samples. Don't resize after init
		util::RingBuffer<Sample> buffer;
		util::Atomic<bool> allBuffered;
		
		///
		/// AudioMgr writes and reads
		///
		
		util::Atomic<SizeType> soundChannelId; // Which channel in Sound
		Smoothed<real32> volumeMul;
		Smoothed<real32> pan;
	};
	
	std::array<AudioChannel, 2> audioChannels; // Only 2 channels per sound (L and R) supported
};

/// Thread-safe OutputStream interface for audio output thread
class DeviceAudioFeed {
public:
	/// Represents OutputChannel
	class Channel {
	public:
		using SampleChunk= util::DynArray<audio::OutputChannel::AudioChannel::Sample>;
		Channel(OutputChannel& ch);
		
		/// If false, following methods shouldn't be called
		bool isActive() const;
		
		/// Maximum sample count which can be read
		SizeType getSampleCount() const;
		
		/// @return Returns multiple chunks of samples. Chunks should be played simultaneously
		///			Chunks are usually from different channels of one sound
		util::DynArray<SampleChunk> readSamples(SizeType request_count);
		
		/// Did last read underrun
		bool didUnderrun() const { return underrun; }
		
		/// If true, no more samples will be added
		bool isAllBuffered() const;
		
		/// If true, there is, and will be no more samples
		bool eos() const;
		
		/// Transfer ownership back to audio manager
		void close();
		
	private:
		OutputChannel* channel;
		bool underrun= false;
	};

	DeviceAudioFeed(OutputChannel* channels, SizeType count);
	SizeType getChannelCount() const { return channelCount; }
	
	Channel getChannel(SizeType i){ return Channel(channels[i]); }
	
private:
	OutputChannel* channels;
	SizeType channelCount;
};

} // audio
} // clover

#endif // CLOVER_AUDIO_OUTPUTCHANNEL_HPP