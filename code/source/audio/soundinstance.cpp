#include "soundinstance.hpp"
#include "sound.hpp"
#include "soundinstancehandle.hpp"
#include "global/env.hpp"
#include "global/event.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace audio {

SoundInstance::SoundInstance(const Sound& _sound, AudioSource& _source)
		: created(false)
		, assignedToChannel(false)
		, finished(false)
		, sound(&_sound)
		, source(&_source)
		, volume(1.0){
}

SoundInstance::~SoundInstance(){
	ensure(!created);
}

void SoundInstance::create(){
	ensure(!created);
	created= true;
	
	for (SizeType i=0; i<sound->getChannelCount(); ++i){
		streams.pushBack(sound->createStream(i));
	}
	
	global::Event e(global::Event::OnSoundInstanceCreate);
	e(global::Event::Object)= this;
	e.send();
}

void SoundInstance::destroy(){
	ensure(created);
	
	created= false;
	
	for (auto& m : streams){
		if (!m.expired())
			sound->destroyStream(m.lock());
	}
	
	streams.clear();
	
	global::Event e(global::Event::OnSoundInstanceDestroy);
	e(global::Event::Object)= this;
	e.send();
}

void SoundInstance::setOnEndCallback(const OnEndCallbackType& cb){
	ensure(!onEndCallback);
	onEndCallback= cb;
}

Sound::ChannelData SoundInstance::getNextSamples(SizeType request_count, SizeType channel_id) {
	if (channel_id >= streams.size()) // If resource is changed, requested channel may be in use even though streams have changed
		return Sound::ChannelData();

	auto ptr= streams[channel_id].lock();
	
	if (!ptr)
		return Sound::ChannelData();
	
	try {
		auto samples= ptr->getNextSamples(request_count);
		return samples;
	}
	catch (resources::ResourceException& e){
		// Corrupted stream or something similar
		
		streams.clear();
		
		global::g_env->resCache->setResourceToErrorState<Sound>(sound->getIdentifierAsString());

		// Create streams for error-sound
		for (SizeType i=0; i<sound->getChannelCount(); ++i){
			streams.pushBack(sound->createStream(i));
		}
	}
	
	return Sound::ChannelData();
	
}

bool SoundInstance::eos(SizeType channel_id) const {
	if(channel_id >= streams.size())
		return true;
	
	auto ptr= streams[channel_id].lock();
	
	if (!ptr)
		return true;
		
	return ptr->hasEnded();
}

} // audio
} // clover
