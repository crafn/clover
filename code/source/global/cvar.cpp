#include "cvar.hpp"
#include "script/script_mgr.hpp"

namespace clover {
namespace global {

CVar::CVar(const util::Str8& n, util::ObjectNode ob)
		: name(n)
		, value(std::move(ob)){
}

util::Str8 CVar::generateString() const {
	return util::Str8("\"") + name + "\"" + " : " + get<util::ObjectNode>().generateText().endNewlineErased();
}

} // global
} // clover
