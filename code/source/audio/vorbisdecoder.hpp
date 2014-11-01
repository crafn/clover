#ifndef CLOVER_AUDIO_VORBISDECODER_HPP
#define CLOVER_AUDIO_VORBISDECODER_HPP

#include "build.hpp"
#include "audiostream_vorbis.hpp"
#include "util/dyn_array.hpp"

#include <vorbis/codec.h>
#include <memory>

namespace clover {
namespace audio {

class VorbisDecoder {
public:
	typedef AudioStream::Sample Sample;
	typedef AudioStream::ChannelData ChannelData;

	VorbisDecoder();
	virtual ~VorbisDecoder();

	/// Gains ownership of data (by moving) if it's valid ogg vorbis bitstream
	/// Throws a resource::ResourceException if data was not ogg
	void preCode(util::DynArray<uint8> data);

	SizeType getChannelCount() const;

	/// Creates a new stream for getting samples
	/// Streams are destroyed also when the decoder is destroyed or reset():ed
	std::weak_ptr<VorbisAudioStream> createStream(SizeType channel_id);
	void destroyStream(const std::shared_ptr<AudioStream>& stream);

	void clear();

private:
	util::DynArray<uint8> data;
	SizeType audioDataBeginIndex;
	bool cleared;

	util::DynArray<std::shared_ptr<VorbisAudioStream>> streams;

	// Makes sure that audio isn't read when data is modified
	boost::mutex accessMutex;

	ogg_stream_state oggStreamState; // Pages to stream
	ogg_sync_state oggSyncState;

	vorbis_info vorbisInfo;
	vorbis_comment vorbisComment;
};

} // audio
} // clover

#endif // CLOVER_AUDIO_VORBISDECODER_HPP
