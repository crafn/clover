#include "outputchannel.hpp"

namespace clover {
namespace audio {

OutputChannel::OutputChannel()
		: state(State::Free)
		, soundInstance(nullptr)
		, audioSource(nullptr)
		, audioReceiver(nullptr){
			
}

OutputChannel::AudioChannel::AudioChannel()
		: allBuffered(false)
		, soundChannelId(soundChannelIdNone)
		, volumeMul(1.0f)
		, pan(0.0f){
}

DeviceAudioFeed::Channel::Channel(OutputChannel& ch)
		: channel(&ch){
}

bool DeviceAudioFeed::Channel::isActive() const {
	return channel->state == OutputChannel::State::Play;
}

SizeType DeviceAudioFeed::Channel::getSampleCount() const {
	auto cmp= [] (	OutputChannel::AudioChannel& ch1,
					OutputChannel::AudioChannel& ch2){
		return ch1.buffer.getUsedCount() < ch2.buffer.getUsedCount();
	};
	
	return std::max_element(channel->audioChannels.begin(),
							channel->audioChannels.end(),
							cmp)->buffer.getUsedCount();
}

auto DeviceAudioFeed::Channel::readSamples(SizeType request_count) -> util::DynArray<SampleChunk> {
	underrun= false;
	
	util::DynArray<SampleChunk> chunks;
	for (auto& audio_ch : channel->audioChannels){
		SizeType read_count= request_count;
		if (read_count > audio_ch.buffer.getUsedCount()){
			if (!audio_ch.allBuffered)
				underrun= true;
			read_count= audio_ch.buffer.getUsedCount();				
		}
		
		chunks.pushBack(audio_ch.buffer.read(read_count));
	}
	return chunks;
}

bool DeviceAudioFeed::Channel::isAllBuffered() const {
	for (const auto& audio_ch : channel->audioChannels){
		if (!audio_ch.allBuffered)
			return false;
	}
	return true;
}

bool DeviceAudioFeed::Channel::eos() const {
	for (const auto& audio_ch : channel->audioChannels){
		if (!audio_ch.allBuffered || audio_ch.buffer.getUsedCount() > 0)
			return false;
	}
	
	return true;
}

void DeviceAudioFeed::Channel::close(){
	ensure(channel->state == OutputChannel::State::Play);
	channel->state= OutputChannel::State::Close;
}

DeviceAudioFeed::DeviceAudioFeed(OutputChannel* channels, SizeType count)
		: channels(channels)
		, channelCount(count){
	ensure(channels != nullptr);
}

} // audio
} // clover