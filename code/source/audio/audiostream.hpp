#ifndef CLOVER_AUDIO_AUDIOSTREAM_HPP
#define CLOVER_AUDIO_AUDIOSTREAM_HPP

#include "build.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace audio {

/// Internal class of the audiosystem
/// Decodes/streams one channel from.. a source
class AudioStream {
public:
	struct Sample {
		real32 amplitude;
	};

	struct ChannelData {
		util::DynArray<Sample> samples;
	};

	AudioStream();
	virtual ~AudioStream();

	virtual ChannelData getNextSamples(SizeType request_count)= 0;

	/// True if no more data to read
	bool hasEnded() const;

protected:
	bool eos;
};

} // audio
} // clover

#endif // CLOVER_AUDIO_AUDIOSTREAM_HPP