#ifndef CLOVER_UTIL_BOOL_WRAP_HPP
#define CLOVER_UTIL_BOOL_WRAP_HPP

#include "build.hpp"

namespace clover {
namespace util {

class BoolWrap {
public:
	BoolWrap():b(false){}

	template <typename T>
	BoolWrap(const T& t){
		if (t){
			b= true;
		}
		else b= false;
	}

	operator bool() const { return b; }

	typedef bool & boolref;
	operator boolref() { return b; }

	template <typename T>
	bool operator=(const T& t){ 
		b= (bool)t;

		return b;
	}

private:
	bool b;
};

} // util
} // clover

#endif // CLOVER_UTIL_BOOL_WRAP_HPP