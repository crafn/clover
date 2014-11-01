#ifndef CLOVER_NODES_SCRIPT_HPP
#define CLOVER_NODES_SCRIPT_HPP

#include "build.hpp"

namespace clover {
namespace script {

class ScriptMgr;

} // script
namespace nodes {

void registerToScript();
void registerInstanceSlots_Part1(script::ScriptMgr& script);
void registerInstanceSlots_Part2(script::ScriptMgr& script);

} // nodes
} // clover

#endif // CLOVER_NODES_SCRIPT_HPP