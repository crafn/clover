#ifndef CLOVER_AUDIORECEIVER_HPP
#define CLOVER_AUDIORECEIVER_HPP

#include "build.hpp"
#include "util/math.hpp"
#include "util/referencecountable.hpp"

/// @todo Replace with util::Mutex
#include <boost/thread/mutex.hpp>

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
	
	mutable boost::mutex accessMutex;
};

} // audio
} // clover

#endif // CLOVER_AUDIORECEIVER_HPP
