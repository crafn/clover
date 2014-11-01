#ifndef CLOVER_NODES_UPDATELINE_HPP
#define CLOVER_NODES_UPDATELINE_HPP

#include "build.hpp"
#include "util/class_preproc.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace nodes {

class NodeInstance;
class NodeInstanceGroup;

/// Manages updating and batching of nodes from several NodeInstanceGroups
class UpdateLine {
public:
	using RawList= util::DynArray<NodeInstance*>;

	UpdateLine()= default;
	UpdateLine(RawList nodes, const NodeInstanceGroup& group);
	DEFAULT_COPY(UpdateLine);
	DEFAULT_MOVE(UpdateLine);

	void append(UpdateLine other);

	/// Makes batching happen
	void sort();

	/// Updates every node
	void run();

private:
	struct NodeInfo {
		NodeInstance* node;
		int32 priority;
	};

	using List= util::DynArray<NodeInfo>;
	List list;
};

} // nodes
} // clover

#endif // CLOVER_NODES_UPDATELINE_HPP
