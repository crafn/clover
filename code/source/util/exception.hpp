#ifndef CLOVER_UTIL_EXCEPTION_HPP
#define CLOVER_UTIL_EXCEPTION_HPP

#include "build.hpp"

#include <stdexcept>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <boost/exception/all.hpp>

namespace clover {
namespace util {

class Exception : public virtual std::exception, public virtual boost::exception {
public:

    Exception(const char8* s, ...);
	Exception(const char8* s, va_list arglist);

    virtual ~Exception() throw();
    virtual const char* what() const throw();

protected:
	Exception();
	void constructor(const char8* s, va_list arglist);

private:
	const char8* error;

};

/// Can't be catched by catch (Exception& e)
class FatalException : Exception, public virtual std::exception, public virtual boost::exception {
public:
	FatalException(const char8* s, ...);
	virtual ~FatalException() throw() {}

protected:
	using Exception::constructor;
};

} // util
} // clover

#endif // CLOVER_UTIL_EXCEPTION_HPP
