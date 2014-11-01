#include "audiostream.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace audio {

AudioStream::AudioStream()
		: eos(false){
}

AudioStream::~AudioStream(){
}

bool AudioStream::hasEnded() const {
	return eos;
}

} // audio
} // clover