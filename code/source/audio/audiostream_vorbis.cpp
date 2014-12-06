#include "audiostream_vorbis.hpp"
#include "vorbisdecoder.hpp"
#include "resources/exception.hpp"
#include "util/string.hpp"
#include "util/ensure.hpp"
#include "util/math.hpp"

namespace clover {
namespace audio {

VorbisAudioStream::VorbisAudioStream(	const uint8* data_begin,
										const uint8* data_end,
										SizeType channel_id,
										vorbis_info& info,
										util::Mutex& access_mutex)
		: accessMutex(&access_mutex)
		, data(data_begin)
		, dataSize(data_end - data_begin)
		, dataIndex(0)
		, channelId(channel_id)
		, needsNewPage(true)
		, needsNewPacket(true)
		, needsNewBlock(true)
		, streamAllocated(false)
		, vorbisInfo(&info){
	ogg_sync_init(&oggSyncState);
}

VorbisAudioStream::~VorbisAudioStream(){
	if (streamAllocated){
		ogg_stream_clear(&oggStreamState);
		vorbis_block_clear(&vorbisBlock);
		vorbis_dsp_clear(&vorbisDspState);
		ogg_sync_clear(&oggSyncState);
	}
}

AudioStream::ChannelData VorbisAudioStream::getNextSamples(SizeType request_count){
	util::LockGuard<util::Mutex> lock(*accessMutex);
	
	ChannelData channeldata;
	if (eos)
		return channeldata;
	
#if 0
	for (SizeType i=0; i<request_count && dataIndex < dataSize; ++i){
		Sample s;
		s.amplitude= sin((real32)dataIndex*util::tau/44100.0 * 500.0); // 500 hz
		channeldata.samples.pushBack(std::move(s));
		
		++dataIndex;
	}
	
	if (dataIndex >= dataSize) eos= true;

	return (channeldata);

#endif

	// Decode audio

	channeldata.samples.reserve(request_count);

	//print(debug::Ch::Audio, debug::Vb::Trivial, "Chan: %i", channelId);
	
	SizeType samples_decoded= 0;
	while (samples_decoded < request_count && !eos){
		bool first_time= dataIndex == 0;
		
		SizeType data_submitted= 0;
		if (needsNewPage){
			data_submitted= submitPageData();
			
			if (data_submitted != 0)
				needsNewPage= false;
			
			// First call
			if (first_time){
				SizeType ret=0;

				ret= ogg_stream_init(&oggStreamState, ogg_page_serialno(&oggPage));
				ensure(!ret);
				
				ret= vorbis_synthesis_init(&vorbisDspState, vorbisInfo);
				ensure(!ret);
				
				ret= vorbis_block_init(&vorbisDspState, &vorbisBlock);
				ensure(!ret);
				
				streamAllocated= true;
			}
			
			ogg_stream_pagein(&oggStreamState, &oggPage);
		}
		
		ensure(oggPage.body); // There has to be a page

		if (needsNewPage && ogg_stream_eos(&oggStreamState) && ogg_page_eos(&oggPage) && dataIndex == dataSize){
			eos= true;
			break;
		}
		
		while(samples_decoded < request_count){
			if (needsNewPacket){
				int32 result= ogg_stream_packetout(&oggStreamState, &oggPacket);
				
				if (result == 0){
					needsNewPage= true; // Next page
					break; 
				}
				else if (result < 0){
					print(debug::Ch::Audio, debug::Vb::Trivial, "Invalid/corrupt ogg packet");
				}
				else {
					
					needsNewPacket= false;
					needsNewBlock= true;
				}
			}

			if (!needsNewPacket) {
				// Process the packet
				
				if (needsNewBlock){
					int32 result=0;
					
					// Packet to block
					if ((result= vorbis_synthesis(&vorbisBlock, &oggPacket)) == 0){
						vorbis_synthesis_blockin(&vorbisDspState, &vorbisBlock);
						needsNewBlock= false;
					}
					else {
						if (result == OV_ENOTAUDIO){
							// Skip the header
						}
						else {
							print(debug::Ch::Audio, debug::Vb::Critical, "Error in synthesis: %i", result);
						}
					}
				}
				
				real32 **rawsamples;
				while (samples_decoded < request_count){
					uint32 sample_count=0; // Sample count of each channel
					sample_count = vorbis_synthesis_pcmout(&vorbisDspState, &rawsamples);
					
					if (sample_count <= 0){
						// New packet needed, not block, because packet is converted to a block at once
						needsNewPacket= true;
						break;
					}
					
					uint32 samples_to_read= sample_count; // Sample count of one channel
					ensure(samples_decoded < request_count);
					if (samples_to_read + samples_decoded >= request_count){
						samples_to_read = request_count - samples_decoded;
					}

					real32* mono= rawsamples[channelId];
					for (SizeType i=0; i<samples_to_read; ++i){
						Sample s;
						s.amplitude= mono[i];
						channeldata.samples.pushBack(std::move(s));
					}
					
					samples_decoded += samples_to_read;
					vorbis_synthesis_read(&vorbisDspState, samples_to_read);
				}
			}
		}
		ensure(dataIndex <= dataSize);
	}

	return channeldata;
}

SizeType VorbisAudioStream::submitPageData(){
	SizeType total= 0;
	
	while (ogg_sync_pageout(&oggSyncState, &oggPage) != 1){
		ensure(dataIndex <= dataSize);
		
		if (dataIndex == dataSize){
			//print(debug::Ch::Audio, debug::Vb::Trivial, "All data written to ogg stream");
			break;
		}
		
		char* buf= ogg_sync_buffer(&oggSyncState, pageSize);
		ensure(buf);
		
		SizeType written=0;
		for (SizeType i=0; i<pageSize && dataIndex < dataSize; ++i){
			buf[i]= data[dataIndex];
			++dataIndex;
			++written;
		}
		
		SizeType ret= ogg_sync_wrote(&oggSyncState, written);
		ensure(ret == 0);
		
		total += written;
	}
	
	ensure(dataIndex <= dataSize);
	return total;
}

} // audio
} // clover