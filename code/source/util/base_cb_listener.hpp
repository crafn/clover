#ifndef CLOVER_UTIL_BASE_CB_LISTENER_HPP
#define CLOVER_UTIL_BASE_CB_LISTENER_HPP

#include "build.hpp"

namespace clover {
namespace util {

class BaseCallbacker;
class BaseCbListener {
public:

	virtual ~BaseCbListener(){}

	/// Stops listening
	virtual void clear() = 0;

protected:
	friend class BaseCallbacker;

	template <typename... Args>
	friend class SingleCallbacker;

	/// BaseCallbacker calls when moved
	virtual void setCallbacker(const BaseCallbacker& c) = 0;
};
	
} // util
} // clover

#endif // CLOVER_UTIL_BASE_CB_LISTENER_HPP