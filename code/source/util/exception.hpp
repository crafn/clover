#ifndef CLOVER_UTIL_EXCEPTION_HPP
#define CLOVER_UTIL_EXCEPTION_HPP

#include "build.hpp"

#include <stdexcept>
#include <cstdarg>
#include <boost/exception/exception.hpp>

namespace clover {
namespace util {

class ENGINE_API Exception	: public virtual std::exception
							, public virtual boost::exception {
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

} // util
} // clover

#endif // CLOVER_UTIL_EXCEPTION_HPP
