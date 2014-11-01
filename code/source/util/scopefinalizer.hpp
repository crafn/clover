#ifndef CLOVER_UTIL_SCOPEFINALIZER_HPP
#define CLOVER_UTIL_SCOPEFINALIZER_HPP

#include "build.hpp"
#include "util/class_preproc.hpp"

#include <functional>

namespace clover {
namespace util {

class ScopeFinalizer {
public:
	using Callback= std::function<void ()>;

	ScopeFinalizer(Callback cb)
		: callback(cb){}
	DELETE_COPY(ScopeFinalizer);
	DEFAULT_MOVE(ScopeFinalizer);
	~ScopeFinalizer(){
		if (callback)
			callback();
	}

private:
	Callback callback;
};

} // util
} // clover

#endif // CLOVER_UTIL_SCOPEFINALIZER_HPP