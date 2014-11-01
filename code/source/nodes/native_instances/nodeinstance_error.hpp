#ifndef CLOVER_NODES_NODEINSTANCE_ERROR_HPP
#define CLOVER_NODES_NODEINSTANCE_ERROR_HPP

#include "build.hpp"
#include "nodeinstance.hpp"

namespace clover {
namespace nodes {

/// Used when NodeInstanceType has invalid NodeInstanceTypeImpl
class ErrorNodeInstance : public NodeInstanceImpl {
public:
	virtual ~ErrorNodeInstance(){}
};

} // nodes
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_ERROR_HPP