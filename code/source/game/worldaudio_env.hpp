#ifndef CLOVER_GAME_WORLDAUDIO_ENV_HPP
#define CLOVER_GAME_WORLDAUDIO_ENV_HPP

#include "build.hpp"
#include "audio/soundinstancehandle.hpp"

namespace clover {
namespace game {

class WorldAudioEnv {
public:
	WorldAudioEnv();
	virtual ~WorldAudioEnv();
	
	void setAmbientTrack(const util::Str8& resource_name);
	
private:
	void onAmbientEnd(const audio::SoundInstanceHandle& h) const;
};

} // game
} // clover

#endif // CLOVER_GAME_WORLDAUDIO_ENV_HPP