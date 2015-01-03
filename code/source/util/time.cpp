#include "time.hpp"
#include "hardware/device.hpp"

namespace clover {
namespace util {

using std::chrono::milliseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

Clock *gRealClock;

/// @todo Remove
PtrTable<Clock> gClockTable;
PtrTable<Timer> gTimerTable;

std::chrono::milliseconds asMilliseconds(double seconds){
	return std::chrono::milliseconds((uint64)(1000.0*seconds + 0.49));
}

std::chrono::microseconds asMicroseconds(double seconds){
	return std::chrono::microseconds((uint64)(1000000.0*seconds + 0.49));
}

std::chrono::nanoseconds asNanoseconds(double seconds){
	return std::chrono::nanoseconds((uint64)(1000000000.0*seconds + 0.49));
}

///
/// Clock
///

void Clock::updateAll(){
	for (uint32 i=0; i<gClockTable.size(); i++){
		if (gClockTable[i] == 0) continue;
		gClockTable[i]->update();
	}
}

Clock::Clock():
	timeFromStart(0),
	frameCount(0),
	deltaTime(0.0),
	fixedDeltaTime(0.0),
	timeScale(1.0),
	paused(false),
	tableIndex(gClockTable.findFreeIndex()){

	gClockTable[tableIndex]= this;

}

Clock::~Clock(){
	gClockTable.remove(tableIndex);
}

void Clock::reset(){

}

void Clock::update(){
	if (paused){
		deltaTime=0;
		return;
	}
	else {

		deltaTime= hardware::gDevice->getFrameTime()*timeScale;
		timeFromStart += deltaTime*clockRes;
		frameCount++;
	}
}

real32 Clock::getDeltaTime() const {
	if (fixedDeltaTime > 0.0)
		return fixedDeltaTime;
		
	return deltaTime;
}

real64 Clock::getTime() const {
	return (real64)timeFromStart/clockRes;
}

void Clock::setTimeScale(real32 scale){
	timeScale= scale;
}
real32 Clock::getTimeScale() const {
	return timeScale;
}

void Clock::setPaused(bool b){
	paused= b;
}

void Clock::toggle(){
	setPaused(!paused);
}

bool Clock::isPaused() const {
	return paused;
}


///
/// Timer
///
/// @todo Separate PerformanceTimer from Timer

Timer::Timer(const util::Str8& n):
	running(false),
	frameCount(0),
	totalTime(0),
	name(n),
	tableIndex(-1){
	
	if (!n.empty()){
		tableIndex= gTimerTable.findFreeIndex();
		gTimerTable[tableIndex]= this;
	}
}

Timer::~Timer(){
	if (tableIndex != -1)
		gTimerTable.remove(tableIndex);
}

util::Str8 Timer::getName(){
	return name;
}


void Timer::run(){
	start= std::chrono::high_resolution_clock::now();
	running= true;
}

void Timer::pause(){
	running= false;
	totalTime += duration_cast<milliseconds>(high_resolution_clock::now() - start).count() / 1000.0;
}
void Timer::nextFrame(){
	++frameCount;
}

void Timer::nextFrames(){
	for (uint32 i=0; i<gTimerTable.size(); ++i){
		if (gTimerTable[i] == 0) continue;
		gTimerTable[i]->nextFrame();
	}
}

void Timer::reset(){
	frameCount= 0;
	totalTime= 0;
}


real32 Timer::getAverage(){
	if (frameCount == 0)
		return 0;//throw Exception("Timer::getAverage(): %s: zero runs", name.c_str());

	return totalTime/frameCount;
}

real32 Timer::getTime(){
	return totalTime;
}

} // util
} // clover
