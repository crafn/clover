#include "audio_mgr.hpp"
#include "audiodevice.hpp"
#include "audiosource.hpp"
#include "audiosourcehandle.hpp"
#include "audioreceiver.hpp"
#include "audioreceiverhandle.hpp"
#include "debug/debugprint.hpp"
#include "global/env.hpp"
#include "global/exception.hpp"
#include "hardware/device.hpp"
#include "soundinstance.hpp"
#include "sound.hpp"
#include "util/atomic.hpp"
#include "util/math.hpp"
#include "util/profiling.hpp"

// For debugging purposes
#include "hardware/keyboard.hpp"

namespace clover {
namespace audio {

/// @todo Rename and expose - these are useful
void scriptPlayGlobalSound(const util::Str8& sound_name, real32 volume, real32 panning){
	audio::AudioSourceHandle as;
	as.assignNewSource(AudioSource::Type::Global);
	as.setVolume(volume);
	as.playSound(sound_name);
}

void scriptPlaySpatialSound(const util::Str8& sound_name, const util::Vec2d& position, real32 volume){
	audio::AudioSourceHandle as;
	as.assignNewSource(AudioSource::Type::Spatial);
	as.setPosition(position);
	as.setVolume(volume);
	as.playSound(sound_name);
}

AudioMgr::AudioMgr()
		: quitFeedThread(false)
		, outputStream(gAudioDevice->getOutputStream())
{
	if (global::g_env.audioMgr == nullptr)
		global::g_env.audioMgr= this;

	feedThread= std::thread(std::bind(&AudioMgr::feedLoop, this));
}

AudioMgr::~AudioMgr()
{
	quitFeedThread= true;
	feedThread.join();

	for (auto& m : soundInstances)
		m.destroy();

	for (auto& m : audioSources)
		m.destroy();

	for (auto& m : audioReceivers)
		m.destroy();

	if (global::g_env.audioMgr == this)
		global::g_env.audioMgr= nullptr;
}

SizeType AudioMgr::getSoundInstanceCount() const {
	return soundInstances.size();
}

SizeType AudioMgr::getAudioSourceCount() const {
	return audioSources.size();
}

SizeType AudioMgr::getFreeChannelCount() const {
	SizeType ret= 0;
	for (auto& m : outputStream.getChannels()){
		if (m.state == OutputChannel::State::Free)
			++ret;
	}
	return ret;
}

SoundInstanceHandle AudioMgr::playSound(const Sound& sound, AudioSource& owner_source){
	soundInstances.pushBack(std::move(SoundInstance(sound, owner_source)));
	soundInstances.back().create();
	owner_source.addSoundInstance(soundInstances.back());
	return (SoundInstanceHandle(&soundInstances.back()));
}

void AudioMgr::update(){
	PROFILE_("audio");
	{
		// Play new sounds

		for (auto it= soundInstances.begin(); it != soundInstances.end(); ++it){
			if (!it->isAssignedToChannel()){
				assignToChannel(it);
			}
		}

		// Remove finished sounds
		
		util::Map<SoundInstance*, SizeType> counts;
		
		for (auto& ch : outputStream.getChannels()){
			if (ch.state == OutputChannel::State::Free)
				continue;
			++counts[ch.soundInstance];
		}
		
		util::LinkedList<std::pair<SoundInstance*, SoundInstance::OnEndCallbackType>> on_end_callbacks;
		for (auto& m : soundInstances){
			if (m.isAssignedToChannel() && counts[&m] == 0 && m.getOnEndCallback())
				on_end_callbacks.pushBack(std::make_pair(&m, m.getOnEndCallback()));
		}
		
		for (auto& m : on_end_callbacks){
			m.second(m.first);
		}
		
		// Recalculate, because callbacks might have triggered new sounds
		counts.clear();
		for (auto& ch : outputStream.getChannels()){
			if (ch.state == OutputChannel::State::Free)
				continue;
			++counts[ch.soundInstance];
		}
		
		auto it= soundInstances.begin();
		
		while (it != soundInstances.end()){
			if (it->isAssignedToChannel() && counts[&(*it)] == 0){
				print(debug::Ch::Audio, debug::Vb::Trivial, "Sound finished: %s", it->getSound().getIdentifierAsString().cStr());
				
				it->destroy();
				it= soundInstances.erase(it);
			}
			else ++it;
		}
	}
	
	{
		// Remove abandoned audiosources
		auto it= audioSources.begin();
		while (it != audioSources.end()){
			if (it->getReferenceCount() == 0 && it->hasFinished()){
				it->destroy();
				it= audioSources.erase(it);
				
			}
			else ++it;
		}
	
	}
	
	{
		// Remove abandoned audioreceivers
		auto it= audioReceivers.begin();
		while (it != audioReceivers.end()){
			if (it->getReferenceCount() == 0){
				it->destroy();
				it= audioReceivers.erase(it);
			}
			else ++it;
		}		
	}
	
	for (auto& m : audioSources){
		m.update();
	}
}


void AudioMgr::assignToChannel(util::LinkedList<SoundInstance>::Iter sound_it){
	ensure(!sound_it->isAssignedToChannel());

	const Sound& sound= sound_it->getSound();
	AudioSource& owner_source= sound_it->getSource();

	SizeType sound_ch_count= sound.getChannelCount();
	SizeType recv_count= audioReceivers.size();
	if (owner_source.getType() == AudioSource::Type::Global)
		recv_count= 1;

	// Find free channels

	util::DynArray<SizeType> outputchannel_ids;
	util::DynArray<OutputChannel>& outputchannels= outputStream.getChannels();

	for (SizeType i= 0; i < outputchannels.size(); ++i){
		if (outputchannel_ids.size() == recv_count) break;

		if (outputchannels[i].state == OutputChannel::State::Free){
			outputchannel_ids.pushBack(i);
		}
	}

	if (outputchannel_ids.size() < recv_count || recv_count == 0){
		return;
	}
	
	sound_it->assignToChannel();
	
	// Assign to channels
	
	SizeType ch_i= 0;
	for (auto& ch_id : outputchannel_ids){
		auto& ch= outputchannels[ch_id];
		ch.soundInstance= &(*sound_it);
		ch.audioSource= &owner_source;

		if (owner_source.getType() == AudioSource::Type::Spatial)
			ch.audioReceiver= &(*(std::next(audioReceivers.begin(), ch_i)));
		else
			ch.audioReceiver= nullptr;
		
		for (SizeType audio_ch_i= 0; audio_ch_i < ch.audioChannels.size(); ++audio_ch_i){
			auto& audio_ch= ch.audioChannels[audio_ch_i];
			
			SizeType sound_channel_id= soundChannelIdNone;
			
			if (audio_ch_i < sound_ch_count)
				sound_channel_id= audio_ch_i;
			
			audio_ch.soundChannelId= sound_channel_id;
			audio_ch.volumeMul.reset();
			audio_ch.pan.reset();
		}
		
		// To ensure that channel parameters are set before activation is seen in feed loop
		//boost::atomic_thread_fence(boost::memory_order_seq_cst);

		outputchannels[ch_id].state= OutputChannel::State::Start;

		++ch_i;
	}

	print(debug::Ch::Audio, debug::Vb::Trivial, "Sound started: %s", sound.getIdentifierAsString().cStr());

}

AudioSourceHandle AudioMgr::createAudioSource(AudioSource::Type type){
	audioSources.pushBack(std::move(AudioSource(type)));
	audioSources.back().create();

	return (AudioSourceHandle(&audioSources.back()));
}

AudioReceiverHandle AudioMgr::createAudioReceiver(){
	audioReceivers.pushBack(std::move(AudioReceiver()));
	audioReceivers.back().create();

	return (AudioReceiverHandle(&audioReceivers.back()));
}

void feedUpdate(OutputStream& stream);

void AudioMgr::feedLoop(){
	while (!quitFeedThread){
		feedUpdate(outputStream);
		hardware::gDevice->sleep(0.0001);
	}
}

struct FeedChannelInfo {
	/// Sample data from sound instance channels
	util::Map<std::pair<SoundInstance*, SizeType>, Sound::ChannelData> channelDataMap;
	/// Amount of samples to be read from a channel of a sound instance
	util::Map<std::pair<SoundInstance*, SizeType>, SizeType> requestCountMap;
};

FeedChannelInfo calcFeedChannelInfo(const util::DynArray<OutputChannel>& channels){
	FeedChannelInfo info;
	
	for (SizeType i=0; i<channels.size(); ++i){
		const OutputChannel& channel= channels[i];
		if (channel.state == OutputChannel::State::Free)
			continue;

		// Make sure that channel parameters are seen correctly for active channel
		//boost::atomic_thread_fence(boost::memory_order_seq_cst);
		
		for (SizeType audio_ch_i= 0; audio_ch_i < channel.audioChannels.size(); ++audio_ch_i){
			const auto& audio_ch= channel.audioChannels[audio_ch_i];
			
			if (audio_ch.soundChannelId == soundChannelIdNone)
				continue;
			
			auto key= std::make_pair(channel.soundInstance, audio_ch.soundChannelId);
			
			SizeType request_count= audio_ch.buffer.size()/2; // Magic number 2. Dunno if the buffer should be filled up or not
			SizeType free_slots= audio_ch.buffer.getFreeCount();
			
			if (request_count > free_slots){
				request_count= free_slots;
			}
			
			// No point requesting more samples than fits in the least crowded one
			if (info.requestCountMap.find(key) == info.requestCountMap.end() || info.requestCountMap[key] < request_count){
				info.requestCountMap[key]= request_count;
			}
		}
	}
	
	return info;
}

/// Samples from sound channel to a single OutputChannel::AudioChannel
/// Called asynchronously
void feedSamples(	Sound::ChannelData& channel_data, SizeType request_count,
					OutputChannel& channel, OutputChannel::AudioChannel& audio_channel){
	ensure_msg(channel_data.samples.size() <= request_count,
		"Got more samples than expected: %i, %i", channel_data.samples.size(), request_count);
	ensure(channel.soundInstance);
	
	const SoundInstance& inst= *channel.soundInstance;
	const Sound& sound= inst.getSound();
	
	real32 pan= 0; // Mono
	if (sound.getChannelCount() == 2){ // Stereo
		if (audio_channel.soundChannelId == 0)
			pan= -1; // Left
		else
			pan= 1; // Right
	}
	
	real32 distance_volmul= 1.0f;
	
	if (channel.audioReceiver){
		// Has a receiver, is from spatial source
		util::Vec2d dif= channel.audioSource->getPosition() - channel.audioReceiver->getPosition();
		pan= util::limited(pan + (real32)dif.x*0.15f , -1.0f, 1.0f);
		
		distance_volmul= 500.0/(500.0 + pow(dif.length(), 3));
	}

	real32 vol= sound.getVolume()*inst.getVolume()*inst.getSource().getVolume()*distance_volmul;

	// Smoothed variables
	audio_channel.pan= pan;
	audio_channel.volumeMul= vol;

	util::DynArray<OutputChannel::AudioChannel::Sample> outputsamples(channel_data.samples.size());

	// Calculate samples for output stream
	for (SizeType s= 0; s < channel_data.samples.size(); ++s){
		outputsamples[s].amplitude= channel_data.samples[s].amplitude*audio_channel.volumeMul;
		outputsamples[s].pan= audio_channel.pan;
	}
	
	ensure(outputsamples.size() == channel_data.samples.size());

	audio_channel.buffer.write(outputsamples);
}

/// Feeds samples for every AudioChannel of one OutputChannel
/// Called asynchronously
void feedChannel(FeedChannelInfo& feed_channel_info, OutputChannel& channel){
	ensure(channel.soundInstance);
		
	SoundInstance& inst= *channel.soundInstance;
	const Sound& sound= inst.getSound();
	
	if (sound.getResourceState() != resources::Resource::State::Loaded &&
		sound.getResourceState() != resources::Resource::State::Error)
		return;
	
	for (SizeType audio_ch_i= 0; audio_ch_i < channel.audioChannels.size(); ++audio_ch_i){
		auto& audio_ch= channel.audioChannels[audio_ch_i];
		
		if (audio_ch.soundChannelId == soundChannelIdNone)
			continue;
		
		auto key= std::make_pair(&inst, audio_ch.soundChannelId);
		SizeType request_count= feed_channel_info.requestCountMap[key];
		
		// No room
		if (request_count == 0)
			continue;

		Sound::ChannelData* channeldata= nullptr;
		
		if (feed_channel_info.channelDataMap.find(key) == feed_channel_info.channelDataMap.end()){
			auto data= inst.getNextSamples(request_count, audio_ch.soundChannelId);
			feed_channel_info.channelDataMap[key]= std::move(data);
		}
		
		channeldata= &feed_channel_info.channelDataMap[key];
		ensure(channeldata);
		ensure(channel.audioSource);
		
		feedSamples(*channeldata, request_count, channel, audio_ch);
		
		// Ensure that there's samples in the buffer before it can be seen as playable in the OutputStream
		//boost::atomic_thread_fence(boost::memory_order_seq_cst);
		
		if (inst.eos(audio_ch.soundChannelId))
			audio_ch.allBuffered= true;
	}
}

/// @return true if every used AudioChannel of 'channel' has sample(s) in 'buffer'
bool samplesForEveryAudioChannel(const OutputChannel& channel){
	for (const auto& audio_ch : channel.audioChannels){
		if (audio_ch.soundChannelId != soundChannelIdNone && audio_ch.buffer.isEmpty())
			return false;
	}
	return true;
}

/// Feeds some audio to output channels
/// Called asynchronously
void feedUpdate(OutputStream& stream){
	util::DynArray<OutputChannel>& channels= stream.getChannels();
	
	auto feed_channel_info= calcFeedChannelInfo(channels);
	
	for (SizeType i= 0; i < channels.size(); ++i){
		OutputChannel& channel= channels[i];
		
		switch (channel.state){
			case OutputChannel::State::Free:
				continue;
			
			case OutputChannel::State::Start:
				// Read samples to buffers before allowing playing
				feedChannel(feed_channel_info, channel);
				if (samplesForEveryAudioChannel(channel))
					channel.state= OutputChannel::State::Play;
			break;
			
			case OutputChannel::State::Play:
				feedChannel(feed_channel_info, channel);
			break;
			
			case OutputChannel::State::Close:
				channel.soundInstance= nullptr;
				channel.audioSource= nullptr;
				channel.audioReceiver= nullptr;
				
				for (auto& audio_ch : channel.audioChannels){
					// Clear buffer
					audio_ch.buffer.read(audio_ch.buffer.getUsedCount());
					audio_ch.allBuffered= true;
				}
				
				channel.state= OutputChannel::State::Free;
			break;
		}
	}
}

} // audio
} // clover
