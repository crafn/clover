#ifndef CLOVER_SCRIPT_EXCEPTIONS_HPP
#define CLOVER_SCRIPT_EXCEPTIONS_HPP

#include "build.hpp"
#include "resources/exception.hpp"

namespace clover {
namespace script {

/// Scripts are resources
class ScriptException : public resources::ResourceException {
public:
	
	ScriptException(const char* str, ...);
	virtual ~ScriptException() throw();
	
private:
};

} // script
} // clover

#endif // CLOVER_SCRIPT_EXCEPTIONS_HPP