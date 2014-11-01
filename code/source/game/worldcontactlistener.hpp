#ifndef CLOVER_GAME_WORLDCOLLISIONLISTENER_HPP
#define CLOVER_GAME_WORLDCOLLISIONLISTENER_HPP

#include "build.hpp"
#include "audio/sound.hpp"
#include "physics/contactlistener.hpp"
#include "util/string.hpp"
#include "util/map.hpp"

namespace clover {
namespace game {

class WorldContactListener : public physics::BaseContactListener {
public:
	virtual void onBeginContact(const physics::Contact&);
	virtual void onEndContact(const physics::Contact&);
	virtual void onPreSolveContact(const physics::Contact&);
	virtual void onPostSolveContact(const physics::PostSolveContact&);

private:
	struct RecentSoundInfo {
		real64 time;
		real64 amplitude;
	};

	util::Map<const audio::Sound*, RecentSoundInfo> recentSounds;

	bool canBePlayed(const audio::Sound& sound, real64 amplitude);
	void notifyPlaying(const audio::Sound& sound, real64 amplitude);
};

} // game
} // clover

#endif // CLOVER_GAME_WORLDCOLLISIONLISTENER_HPP