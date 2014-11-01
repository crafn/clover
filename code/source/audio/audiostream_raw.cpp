#include "audiostream_raw.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace audio {

RawAudioStream::RawAudioStream(const real32* samples_begin, const real32* samples_end)
		: begin(samples_begin)
		, end(samples_end)
		, current(samples_begin){
}

RawAudioStream::~RawAudioStream(){
}

AudioStream::ChannelData RawAudioStream::getNextSamples(SizeType request_count){
	ChannelData ret;
	
	SizeType read_count= request_count;
	ensure(end >= current);
	if (read_count > (SizeType)(end - current))
		read_count= end - current;
	
	ret.samples.resize(read_count);
	
	for (SizeType i=0; i < request_count; ++i){
		if (current == end){
			eos= true;
			break;
		}
		
		Sample s;
		s.amplitude= *current;
		
		ret.samples[i]= s;
		++current;
	}
	
	return (ret);
	
}

} // audio
} // clover