#ifndef CLOVER_AUDIO_AUDIOSTREAM_RAW_HPP
#define CLOVER_AUDIO_AUDIOSTREAM_RAW_HPP

#include "build.hpp"
#include "audiostream.hpp"

namespace clover {
namespace audio {

/// A stream of floating point samples
class RawAudioStream : public AudioStream {
public:
	/// @param samples_begin Pointer (iterator) to the begin of sample-array
	/// @param samples_begin Pointer (iterator) to the end of sample-array
	/// @param accessMutex Mutex to prevent reading in feed thread when sound is changing
	RawAudioStream(const real32* samples_begin, const real32* samples_end);
	virtual ~RawAudioStream();

	virtual ChannelData getNextSamples(SizeType request_count);
	
private:
	const real32* begin,
				* end,
				* current;
};

} // audio
} // clover

#endif // CLOVER_AUDIO_AUDIOSTREAM_RAW_HPP