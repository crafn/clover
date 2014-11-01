#ifndef CLOVER_NODES_COMPOSITIONNODE_UI_HPP
#define CLOVER_NODES_COMPOSITIONNODE_UI_HPP

#include "build.hpp"

namespace clover {
namespace nodes {

class CompositionNodeType;

class CompositionNodeUi {
public:
	CompositionNodeUi(CompositionNodeType&);
	
private:
	CompositionNodeType* type;
};

} // nodes
} // clover

#endif // CLOVER_NODES_COMPOSITIONNODE_UI_HPP