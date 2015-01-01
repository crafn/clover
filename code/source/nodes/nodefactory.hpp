#ifndef CLOVER_NODES_NODEFACTORY_HPP
#define CLOVER_NODES_NODEFACTORY_HPP

#include "build.hpp"

namespace clover {
namespace nodes {

class CompositionNodeLogic;
class NodeInstance;

NodeInstance* createNodeInstanceNativeLogic(const util::Str8& type_string);
CompositionNodeLogic* createCompositionNodeLogic(const util::Str8& type_string);

} // nodes
} // clover

#endif // CLOVER_NODES_NODEFACTORY_HPP
