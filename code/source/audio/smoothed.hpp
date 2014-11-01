#ifndef CLOVER_AUDIO_SMOOTHED_HPP
#define CLOVER_AUDIO_SMOOTHED_HPP

#include "build.hpp"
#include "util/math.hpp"

namespace clover {
namespace audio {

/// Prevents sudden changes
template <typename T>
class Smoothed {
public:
	Smoothed():
		value(0),
		target(0),
		delta(0.005),
		assigned(false){
	}

	Smoothed(const T& v):
		value(v),
		target(0),
		delta(0.005),
		assigned(true){
	}
	
	void reset(){
		assigned= false;
	}

	bool isAssigned() const {
		return assigned;
	}

	Smoothed& operator=(const T& t){
		if (assigned)
			target= t;
		else {
			value= t;
			target= t;
			assigned= true;
		}
		
		return *this;
	}

	operator T(){
		advance();
		return value;
	}

	void setDelta(const T& t){
		delta= t;
	}

	void set(const T& t){
		value= t;
		target= t;
		assigned= true;
	}

	const T& get() const { return value; }

	const T& getTarget() const { return target; }

	void advance(){
		ensure(assigned);

		if (util::abs(value-target) < delta)
			value= target;
		else if (value < target)
			value += delta;
		else if (value > target)
			value -= delta;

	}
	
private:
	T value, target, delta;
	bool assigned;
};

} // audio
} // clover

#endif // CLOVER_AUDIO_SMOOTHED_HPP