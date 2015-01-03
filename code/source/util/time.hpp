#ifndef CLOVER_UTIL_TIME_HPP
#define CLOVER_UTIL_TIME_HPP

#include "build.hpp"
#include "math.hpp"
#include "ptrtable.hpp"
#include "string.hpp"

#include <ctime>
#include <chrono>

namespace clover {
namespace util {

std::chrono::milliseconds asMilliseconds(double seconds);
std::chrono::microseconds asMicroseconds(double seconds);
std::chrono::nanoseconds asNanoseconds(double seconds);

/// Measures time between frames
class Clock {
public:
	static void updateAll();

	Clock();
	virtual ~Clock();

	void reset();
	void update();

	real32 getDeltaTime() const;
	real64 getTime() const;

	void setTimeScale(real32 scale);
	real32 getTimeScale() const;
	
	/// Sets return value of getDeltaTime
	void setFixedDeltaTime(real32 dt){ fixedDeltaTime= dt; }
	
	/// Removes fixed time step
	void unsetFixedDeltaTime(){ fixedDeltaTime= 0.0; }

	void setPaused(bool b);
	void toggle();

	bool isPaused() const;
	
private:
	static const int32 clockRes= 10000;

	uint32 timeFromStart;
	uint32 frameCount;

	real32 deltaTime;
	real32 fixedDeltaTime; // Not used if <= 0.0
	real32 timeScale;

	bool paused;
	
	int32 tableIndex;
};

class Timer {
public:
	Timer(const util::Str8& name= util::Str8(""));
	virtual ~Timer();

	util::Str8 getName();

	void run();
	bool isRunning() const { return running; }
	void pause();

	void nextFrame();

	static void nextFrames();

	void reset();

	// Palauttaa start():n ja stop():n välisen ajan (keskiarvo, jos suoritettu monta kertaa peräkkäin ilman resettiä)
	real32 getAverage();
	
	// Palauttaa kokonaisajan, ei saa kutsua pausettamatta
	real32 getTime();

private:
	bool running;
	int32 frameCount;
	real32 totalTime;

	std::chrono::high_resolution_clock::time_point start;

	util::Str8 name;

	int32 tableIndex;
};

/// @todo This really shouldn't be global.
extern Clock *gRealClock;

} // util
} // clover

#endif // CLOVER_UTIL_TIME_HPP
