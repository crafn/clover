#ifndef CLOVER_SCRIPT_CONTEXT_HPP
#define CLOVER_SCRIPT_CONTEXT_HPP

#include "build.hpp"
#include "object.hpp"
#include "function.hpp"
#include "util/tuple.hpp"
// .tpp
#include "debug/debugprint.hpp"
#include "util/ensure.hpp"
#include "util.hpp"

// .tpp
#include <angelscript.h>
#include <type_traits>

class asIScriptContext;

namespace clover {
namespace script {

class Module;

/// Handle to AngelScript context
class Context {
public:
	Context();
	Context(Context&& other);
	Context(const Context& other);
	virtual ~Context();
	
	Object instantiateObject(const ObjectType& type);
	
	/// Executes a string of code
	void execute(const util::Str8& string);
	
	/// Returns true if context can be used and/or executed
	bool isFree() const;
	
	template <typename... Args>
	void setArguments(util::Tuple<Args...>& args);
	
	/// Sets arguments and executes
	template <typename F>
	typename FunctionEntry<F>::ReturnValue execute(FunctionEntry<F>& f);
	
	template <typename F>
	typename FunctionEntry<F>::ReturnValue execute(FunctionEntry<F>&& f);
	
	template <typename F>
	typename FunctionEntry<F>::ReturnValue execute(Object& o, FunctionEntry<F>& f);
	
	template <typename F>
	typename FunctionEntry<F>::ReturnValue execute(Object& o, FunctionEntry<F>&& f);
	
private:
	asIScriptContext* context;
	
	template <typename F>
	void prepare(FunctionEntry<F>& f);
	void executeImpl();
	void executionErrorCheck(int32 ret);
	
	/// Recursive
	template <SizeType N, typename... Args>
	void setArguments(const util::Tuple<Args...>& args);
	
	template <typename T>
	struct ValueTranslator;

	template <SizeType N>
	struct ArgumentListSetter;
};

#include "context.tpp"

} // script
} // clover

#endif // CLOVER_SCRIPT_CONTEXT_HPP