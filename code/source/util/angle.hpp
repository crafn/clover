#ifndef CLOVER_UTIL_ANGLE_HPP
#define CLOVER_UTIL_ANGLE_HPP

#include "build.hpp"

namespace clover {
namespace util {

/// @todo
template <typename T>
class Angle {
public:
	Angle(T v=0): value(v){
	}
	
private:
	T value;
};

} // util
} // clover

#endif // CLOVER_UTIL_ANGLE_HPP