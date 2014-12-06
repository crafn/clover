#ifndef CLOVER_AUDIO_SOUND_HPP
#define CLOVER_AUDIO_SOUND_HPP

#include "build.hpp"
#include "audiostream.hpp"
#include "resources/resource.hpp"
#include "vorbisdecoder.hpp"
#include "util/mutex.hpp"

namespace clover {
namespace audio {

class Sound;

} // audio
namespace resources {

template <>
struct ResourceTraits<audio::Sound> {
	DECLARE_RESOURCE_TRAITS(audio::Sound, String)
	
	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"),
							AttributeDef::Path("file"),
							AttributeDef::Real("volume", false, 0.0, 1.0))
	
	typedef SubCache<audio::Sound> SubCacheType;

	static util::Str8 typeName(){ return "Sound"; }
	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static util::Str8 identifierKey(){ return "name"; }

	static bool expired(const audio::Sound&){ return false; }
};

} // resources
namespace audio {

class Sound : public resources::Resource {
public:
	DECLARE_RESOURCE(Sound)

	typedef AudioStream::Sample Sample;
	typedef AudioStream::ChannelData ChannelData;
	
	static constexpr SizeType Channel_Left= 0;
	static constexpr SizeType Channel_Right= 1;

	Sound();
	virtual ~Sound();

	virtual void resourceUpdate(bool load, bool force=true);
	
	/// Called asynchronously (SoundInstance::getNextSamples(..))
	virtual void createErrorResource();
	SizeType getChannelCount() const;
	
	/// Creates and returns pointer to a stream which can be used to read samples
	/// Stream is destroyed when sound's audio data is changed (file modified etc.)
	/// Called asynchronously (SoundInstance::getNextSamples(..))
	std::weak_ptr<AudioStream> createStream(SizeType channel_id) const;
	void destroyStream(const std::shared_ptr<AudioStream>& stream) const;
	
	/// Called asynchronously
	real32 getVolume() const { return volume; }
	
private:
	void load(const util::Str8& filename);
	void unload();
	
	void onFileChange();
	
	uint32 channelCount;
	
	// Because of threading
	real32 volume;
	util::Str8 path;

	mutable util::Mutex accessMutex;
	
	// These could be moved to AudioMgr or something to get rid of mutables
	
	mutable VorbisDecoder decoder; // Used if ogg
	
	mutable util::DynArray<std::shared_ptr<AudioStream>> errorStreams; // Used if error
	util::DynArray<real32> errorData; // Used if error
	
	RESOURCE_ATTRIBUTE(String, nameAttribute);
	RESOURCE_ATTRIBUTE(Path, fileAttribute);
	RESOURCE_ATTRIBUTE(Real, volumeAttribute);
};

} // audio
} // clover

#endif // CLOVER_AUDIO_SOUND_HPP