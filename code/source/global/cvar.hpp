#ifndef CLOVER_GLOBAL_CVAR_HPP
#define CLOVER_GLOBAL_CVAR_HPP

#include "build.hpp"
#include "script/reference.hpp"
#include "util/dyn_array.hpp"
#include "util/callbacker.hpp"
#include "util/objectnode.hpp"
#include "util/string.hpp"
#include "util/traits.hpp"

namespace clover {
namespace global {

/// Configuration variable which can be accessed from script, console and code
/// Named CVar because it fits and most people are probably more familiar with CVar than e.g. CfgVar
/// Stores serialized data, not real instances of variable types
/// @warning	CfgVars should be destroyed after script systems have been destroyed,
///				because global properties can't be unregistered
class CVar : public script::NoCountReference {
public:
	/// @param name e.g. "visual::hideGui"
	/// @param ob A (reference-like) node from a parsed cfg file
	CVar(const util::Str8& name, util::ObjectNode ob);
	CVar(const CVar&)= delete;
	CVar(CVar&&)= default;

	CVar& operator=(const CVar&)= delete;
	CVar& operator=(CVar&&)= default;

	template <typename T>
	void set(const T& v);

	template <typename T>
	T get() const;

	/// @return String: '"name" : value'
	util::Str8 generateString() const;

	/// Internals of cfg system
	void setObjectNode(util::ObjectNode ob){ value= std::move(ob); }

private:
	util::Str8 name;
	util::ObjectNode value;
};

template <typename T>
void CVar::set(const T& v){
	value.setValue(v);
}

template <typename T>
T CVar::get() const {
	return value.getValue<T>();
}

} // global
namespace util {

template <>
struct TypeStringTraits<global::CVar> {
	static util::Str8 type(){ return "::CVar"; }
};

} // util
} // clover

#endif // CLOVER_GLOBAL_CVAR_HPP
