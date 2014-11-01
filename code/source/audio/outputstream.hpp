#ifndef CLOVER_AUDIO_OUTPUTSTREAM_HPP
#define CLOVER_AUDIO_OUTPUTSTREAM_HPP

#include "build.hpp"
#include "outputchannel.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace audio {

/// Holds data that comes out from speakers
/// Be careful when managing channels, they're being modified/read in 3 threads (main, feed, device output)
class OutputStream {
public:
	OutputStream(SizeType channel_count, SizeType buffer_sample_count)
			: channels(channel_count){ init(buffer_sample_count); }

	/// @todo Remove direct access to channels and provide different feeds for different threads
	util::DynArray<OutputChannel>& getChannels(){ return channels; }
	const util::DynArray<OutputChannel>& getChannels() const { return channels; }

	/// Interface for audio device thread
	DeviceAudioFeed getDeviceAudioFeed(){ return DeviceAudioFeed(&channels[0], channels.size()); }

private:
	void init(SizeType buffer_sample_count){
		for (auto& ch : channels){
			for (auto& audio_ch : ch.audioChannels)
				audio_ch.buffer.resize(buffer_sample_count);
		}
	}

	util::DynArray<OutputChannel> channels;
};

} // audio
} // clover

#endif // CLOVER_AUDIO_OUTPUTSTREAM_HPP