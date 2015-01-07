#include "vorbisdecoder.hpp"
#include "debug/print.hpp"
#include "util/ensure.hpp"
#include "global/exception.hpp"
#include "resources/exception.hpp"

namespace clover {
namespace audio {

VorbisDecoder::VorbisDecoder()
		: audioDataBeginIndex(0)
		, cleared(true){
}

VorbisDecoder::~VorbisDecoder(){
	clear();
}

void VorbisDecoder::preCode(util::DynArray<uint8> data){
	util::LockGuard<util::Mutex> lock(accessMutex);
	
	if (!cleared)
		clear();
		
	ogg_packet	oggPacket; // Raw data
	ogg_page	oggPage; // Contains packets
	
	
	// Ogg checks

	ogg_sync_init(&oggSyncState);
	cleared= false;
	
	ensure(data.size() > 4096);
	
	char* buffer= ogg_sync_buffer(&oggSyncState, 4096);
	uint32 dataIndex=0;
	for (uint32 i=0; i<4096; ++i){
		buffer[i]= data[dataIndex];
		++dataIndex;
	}
	ogg_sync_wrote(&oggSyncState, 4096);
	
	bool was_data_ogg= true;
	
	if (ogg_sync_pageout(&oggSyncState, &oggPage) != 1){
		was_data_ogg= false;
	}
	
	ogg_stream_init(&oggStreamState, ogg_page_serialno(&oggPage));
	
	// Vorbis-specific
	
	vorbis_info_init(&vorbisInfo);
	vorbis_comment_init(&vorbisComment);
	
	if (!was_data_ogg){
		// clear() deinits all ogg stuff, so better have them initialized regardless of an error
		throw resources::ResourceException("Data not ogg");
	}
	
	if (ogg_stream_pagein(&oggStreamState, &oggPage) < 0){
		throw resources::ResourceException("Error reading first page");
	}
	
	if (ogg_stream_packetout(&oggStreamState, &oggPacket) != 1){
		throw resources::ResourceException("Error reading header packet");
	}
	
	if (vorbis_synthesis_headerin(&vorbisInfo, &vorbisComment, &oggPacket) < 0){
		throw resources::ResourceException("Ogg doesn't contain Vorbis");
	}
	
	// It's vorbis!
	
	audioDataBeginIndex= 0;
	
	// Header
	uint32 i=0;
	while (i<3){
		while (i<3){
			int32 ret= ogg_sync_pageout(&oggSyncState, &oggPage);
			if (ret == 0) break; // Moar data needed
			
			if (ret == 1){
				ogg_stream_pagein(&oggStreamState, &oggPage);
				
				
				while (i<3){
					// Submit packets of vorbis header
					// 0: Info, 1: Comments, 2: Codebooks
					ret= ogg_stream_packetout(&oggStreamState, &oggPacket);
					
					if (ret == 0) break;
					
					if (ret < 0){
						// Corruption
						throw resources::ResourceException("Vorbis header is corrupt");
					}
					//print(debug::Ch::Audio, debug::Vb::Trivial, "VorbisDecoder::preCode(..): packet size: %i", oggPacket.bytes);
					audioDataBeginIndex += oggPacket.bytes;
					vorbis_synthesis_headerin(&vorbisInfo, &vorbisComment, &oggPacket);
					++i;
					
				}
			}
		}
		buffer= ogg_sync_buffer(&oggSyncState, 4096);
	
		for (uint32 a=0; a<4096 && dataIndex < data.size(); ++a){
			buffer[a]= data[dataIndex];
			++dataIndex;
		}
		
		ogg_sync_wrote(&oggSyncState, 4096);
		
	}
	
	// Comments
	char **ptr= vorbisComment.user_comments;
	while (*ptr){
		print(debug::Ch::Audio, debug::Vb::Trivial, "%s", *ptr);
		++ptr;
	}
	print(debug::Ch::Audio, debug::Vb::Trivial, "Channels: %d, Rate: %ld", vorbisInfo.channels, vorbisInfo.rate);
	print(debug::Ch::Audio, debug::Vb::Trivial, "Encoded by: %s", vorbisComment.vendor);
	
	// Gain ownership
	this->data= std::move(data);
}

SizeType VorbisDecoder::getChannelCount() const {
	ensure(!data.empty());
	return vorbisInfo.channels;
}

std::weak_ptr<VorbisAudioStream> VorbisDecoder::createStream(SizeType channel_id){
	ensure(!data.empty());
	ensure(data.size() >= audioDataBeginIndex);
	
	//print(debug::Ch::Audio, debug::Vb::Trivial, "createStream(..): audioDataBeginIndex: %i, size: %i", audioDataBeginIndex, data->size() - audioDataBeginIndex);
	
	streams.pushBack(std::shared_ptr<VorbisAudioStream>(
		new VorbisAudioStream(
			//&data[audioDataBeginIndex],
			//data.end() - audioDataBeginIndex,
			&data[0], // Stream will skip the header.. Otherwise stream would need to copy current page and stream state
			&data[0] + data.size(),
			channel_id,
			vorbisInfo,
			accessMutex)));
	
	return streams.back();
}

void VorbisDecoder::destroyStream(const std::shared_ptr<AudioStream>& stream){
	auto it= streams.begin();
	while (it != streams.end()){
		
		if (*it == stream){
			it= streams.erase(it);
			break;
		}
		else ++it;
	}
}

void VorbisDecoder::clear(){
	if (cleared) return;
	
	util::LockGuard<util::Mutex> lock(accessMutex);
	
	streams.clear();
	
	ogg_stream_clear(&oggStreamState);
	
	vorbis_comment_clear(&vorbisComment);
	vorbis_info_clear(&vorbisInfo);
	
	ogg_sync_clear(&oggSyncState);
	
	data= 0;
	audioDataBeginIndex= 0;
	cleared= true;
}

} // audio
} // clover