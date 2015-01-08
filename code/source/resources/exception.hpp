#ifndef CLOVER_RESOURCES_EXCEPTIONS_HPP
#define CLOVER_RESOURCES_EXCEPTIONS_HPP

#include "build.hpp"
#include "global/exception.hpp"

namespace clover {
namespace resources {

class ENGINE_API ResourceException : public global::Exception {
public:
	ResourceException(const char8* s, ...);
	virtual ~ResourceException() throw();
	
protected:
	ResourceException();
	using global::Exception::constructor;
};

} // resources
} // clover

#endif // CLOVER_RESOURCES_EXCEPTIONS_HPP