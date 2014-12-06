#ifndef CLOVER_AUDIORECEIVER_HPP
#define CLOVER_AUDIORECEIVER_HPP

#include "build.hpp"
#include "util/math.hpp"
#include "util/mutex.hpp"
#include "util/referencecountable.hpp"

namespace clover {
namespace audio {

/// Internal class of the audiosystem
/// Needed for output of spatial sounds
class AudioReceiver : public util::ReferenceCountable {
public:
	AudioReceiver();
	AudioReceiver(AudioReceiver&& other);
	virtual ~AudioReceiver();
	
	void create();
	void destroy();

	void setPosition(const util::Vec2d& pos);
	
	// Called asynchronously
	util::Vec2d getPosition() const;
	
private:
	bool created;
	util::Vec2d position;
	real32 lowpass;
	
	mutable util::Mutex accessMutex;
};

} // audio
} // clover

#endif // CLOVER_AUDIORECEIVER_HPP
