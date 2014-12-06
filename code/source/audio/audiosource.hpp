#ifndef CLOVER_AUDIO_AUDIOSOURCE_HPP
#define CLOVER_AUDIO_AUDIOSOURCE_HPP

#include "build.hpp"
#include "global/eventreceiver.hpp"
#include "soundinstancehandle.hpp"
#include "util/math.hpp"
#include "util/mutex.hpp"
#include "util/referencecountable.hpp"

namespace clover {
namespace audio {

class AudioEntity;
class AudioMgr;

/// Internal class of the audio system
class AudioSource : public global::EventReceiver, public util::ReferenceCountable {
public:
	enum class Type {
		Global, // Volume is same for every listener
		Spatial // Volume depends on listener position
	};

	AudioSource(Type type_);
	AudioSource(AudioSource&& other);
	virtual ~AudioSource();
	
	void create();
	void destroy();
	
	virtual void onEvent(global::Event& e);

	SoundInstanceHandle playSound(const Sound& s);
	
	void addSoundInstance(SoundInstance& h);

	void setPosition(const util::Vec2d& pos);
	// Called asynchronously
	Type getType() const { return type; }
	util::Vec2d getPosition() const;
	
	//distance meinaa sädettä, jonka sisällä kuulijat ovat
	void update();
	bool hasFinished() const;
	
	// Called asynchronously
	void setVolume(real32 vol){ volume= vol; }
	real32 getVolume() const { return volume; }
	
private:
	bool created;
	Type type;
	
	mutable util::Mutex accessMutex;
	
	util::Vec2d position;
	real32 volume;
	
	util::DynArray<SoundInstance*> soundInstances;
};

} // audio
} // clover

#endif // CLOVER_AUDIO_AUDIOSOURCE_HPP