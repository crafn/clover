#ifndef CLOVER_AUDIO_AUDIOSTREAM_VORBIS_HPP
#define CLOVER_AUDIO_AUDIOSTREAM_VORBIS_HPP

#include "build.hpp"
#include "audiostream.hpp"

/// @todo Replace with util::Mutex
#include <boost/thread/mutex.hpp>
#include <vorbis/codec.h>

namespace clover {
namespace audio {

class VorbisAudioStream : public AudioStream {
public:
	/// @param data_begin Pointer to the audio page in vorbis
	/// @param data_begin Pointer to the end of audio data
	/// @param accessMutex Mutex to prevent reading in feed thread when sound is changing
	VorbisAudioStream(
			const uint8* data_begin,
			const uint8* data_end,
			SizeType channel_id,
			vorbis_info& info,
			boost::mutex& accessMutex);
	virtual ~VorbisAudioStream();

	virtual ChannelData getNextSamples(SizeType request_count);
	
private:
	/// @todo Fix: changing this to bigger will make short sounds not to play
	static constexpr SizeType pageSize= 1024;
	
	SizeType submitPageData();
	
	boost::mutex* accessMutex;
	const uint8* data;
	SizeType dataSize;
	SizeType dataIndex;
	SizeType channelId;
	bool needsNewPage;
	bool needsNewPacket;
	bool needsNewBlock;
	bool streamAllocated;
	
	ogg_packet	oggPacket; // Raw data
	ogg_page	oggPage; // Contains packets
	ogg_sync_state oggSyncState;
	ogg_stream_state oggStreamState;
	
	vorbis_info* vorbisInfo;
	vorbis_dsp_state vorbisDspState; // Decoder state
	vorbis_block vorbisBlock;
};
	
} // audio
} // clover

#endif // CLOVER_AUDIO_AUDIOSTREAM_VORBIS_HPP
