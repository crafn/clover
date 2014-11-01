#ifndef CLOVER_NODES_NODEFACTORY_HPP
#define CLOVER_NODES_NODEFACTORY_HPP

#include "build.hpp"

namespace clover {
namespace script {
	class Module;
}

namespace nodes {

class CompositionNodeLogic;
class NodeInstance;

class NodeFactory {
public:
	static NodeInstance* createNodeInstanceNativeLogic(const util::Str8& type_string);
};

} // nodes
} // clover

#endif // CLOVER_NODES_NODEFACTORY_HPP