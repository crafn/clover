#include "sound.hpp"
#include "audiostream_raw.hpp"
#include "global/file.hpp"

namespace clover {
namespace audio {

Sound::Sound()
		: channelCount(0)
		, volume(1.0)
		, INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(fileAttribute, "file", "")
		, INIT_RESOURCE_ATTRIBUTE(volumeAttribute, "volume", 0.75){
	
	volumeAttribute.setOnChangeCallback([&] (){
		util::LockGuard<util::Mutex> lock(accessMutex);
		volume= volumeAttribute.get();
	});
	
	fileAttribute.setOnChangeCallback([&] (){
		{
			util::LockGuard<util::Mutex> lock(accessMutex);
			path= fileAttribute.get().value();
		}
		
		if (getResourceState() != State::Uninit)
			onFileChange();
	});
	
	fileAttribute.get().setOnFileChangeCallback([&] (resources::PathAttributeValue::FileEvent f){
		if (getResourceState() != State::Uninit)
			onFileChange();
	});
		
}

Sound::~Sound(){
}

void Sound::resourceUpdate(bool load, bool force){
	if (load){
		try {
			
			this->load(fileAttribute.get().whole());

			setResourceState(State::Loaded);
		}
		catch(resources::ResourceException e){
			createErrorResource();
		}
	}
	else {

		unload();
		setResourceState(State::Unloaded);
	}
}

void Sound::createErrorResource(){
	util::LockGuard<util::Mutex> lock(accessMutex);
	
	errorStreams.clear();
	decoder.clear();

	errorData.resize(2048*8);

	for (uint32 i=0; i<errorData.size(); i++){
		
		real32 mul= 1.0f - (real32)i/errorData.size();
		
		if (i % 128 < 64){ // Square
			errorData[i]= -0.05*mul;
		}
		else errorData[i]= 0.05*mul;

		//errorData[i] += (real32)(i % 512)/4048.0*mul; // Saw
	}

	channelCount= 1;

	setResourceState(State::Error);

}

SizeType Sound::getChannelCount() const {
	util::LockGuard<util::Mutex> lock(accessMutex);
	return channelCount;
}

std::weak_ptr<AudioStream> Sound::createStream(SizeType channel_id) const {
	util::LockGuard<util::Mutex> lock(accessMutex);
	
	if (getResourceState() == State::Error){
		errorStreams.pushBack(std::shared_ptr<AudioStream>(
			new RawAudioStream(&errorData[0], &errorData[0]+errorData.size())));
		
		return errorStreams.back();			
	}
	
	return decoder.createStream(channel_id);
}

void Sound::destroyStream(const std::shared_ptr<AudioStream>& stream) const {
	util::LockGuard<util::Mutex> lock(accessMutex);
	
	if (getResourceState() == State::Error){
		for (auto it= errorStreams.begin(); it != errorStreams.end(); ++it){
			if (*it == stream){
				errorStreams.erase(it);
				break;
			}
		}
	}
	else {
		decoder.destroyStream(stream);
	}
}

void Sound::load(const util::Str8& filename){
	util::LockGuard<util::Mutex> lock(accessMutex);
	
	errorData.clear();

	util::DynArray<uint8> data= global::File::readAll(filename);

	if (data.empty())
		throw resources::ResourceException(util::Str8::format("File empty: %s", filename.cStr()).cStr());

	decoder.preCode(std::move(data));
	channelCount= decoder.getChannelCount();
}

void Sound::unload(){
	util::LockGuard<util::Mutex> lock(accessMutex);
	
	decoder.clear();
	errorStreams.clear();
	errorData.clear();
	
	channelCount=0;
}

void Sound::onFileChange(){
	setResourceState(State::Unloaded);
	unload();
}

} // audio
} // clover