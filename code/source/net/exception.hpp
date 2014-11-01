#ifndef CLOVER_NET_EXCEPTIONS_HPP
#define CLOVER_NET_EXCEPTIONS_HPP

#include "build.hpp"
#include "global/exception.hpp"

namespace clover {
namespace net {

class NetException : public global::Exception {
public:
	NetException(const char8* s, ...);
	virtual ~NetException() throw() {}
	
protected:
	NetException(){}
	using global::Exception::constructor;
};

} // net
} // clover

#endif // CLOVER_NET_EXCEPTIONS_HPP