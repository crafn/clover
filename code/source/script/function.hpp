#ifndef CLOVER_SCRIPT_FUNCTION_HPP
#define CLOVER_SCRIPT_FUNCTION_HPP

#include "build.hpp"
#include "util/tuple.hpp"
// .tpp
#include "util/ensure.hpp"

#include <angelscript.h>

class asIScriptFunction;

namespace clover {
namespace script {	

template <typename F>
class FunctionEntry;

template <typename F>
class Function;

/// Defines everything context needs to know to call a script function
template <typename R, typename... Args>
class FunctionEntry<R(Args...)> {
public:
	typedef R ReturnValue;
	
	FunctionEntry(asIScriptFunction& func, Args... args);
	FunctionEntry(const FunctionEntry& other)= delete;
	FunctionEntry(FunctionEntry&& other);
	virtual ~FunctionEntry();
	
	FunctionEntry& operator=(const FunctionEntry& other)= delete;
	FunctionEntry& operator=(FunctionEntry&& other);
	
	util::Tuple<Args...>& getArguments(){ return arguments; }
	const util::Tuple<Args...>& getArguments() const { return arguments; }
	asIScriptFunction& getAsFunction() const { ensure(function); return *function; }
	
private:
	util::Tuple<Args...> arguments;
	asIScriptFunction* function;
};

/// A handle to a function of a script module
template <typename R, typename... Args>
class Function<R(Args...)> {
public:
	Function(); // Invalid state
	Function(asIScriptFunction& func);
	Function(const Function& other);
	Function(Function&& other);
	virtual ~Function();
	
	Function& operator=(const Function& other);
	Function& operator=(Function&& other);
	
	FunctionEntry<R(Args...)> operator()(Args...) const;
	
	bool isGood() const { return function != nullptr; }
	
private:
	asIScriptFunction* function;
};

#include "function.tpp"

} // script
} // clover

#endif // CLOVER_SCRIPT_FUNCTION_HPP