#include "updateline.hpp"
#include "nodeinstance.hpp"
#include "nodetype.hpp"
#include "util/profiling.hpp"

namespace clover {
namespace nodes {

UpdateLine::UpdateLine(RawList nodes, const NodeInstanceGroup& group){
	PROFILE();
	
	list.resize(nodes.size());

	int32 cur_priority= std::numeric_limits<int32>::max();
	bool prev_was_batched= false;
	for (SizeType i= 0; i < nodes.size(); ++i){
		if (nodes[i]->isBatched()){
			// *2 so that non-batched ones can always have unique
			// priority between two batches
			cur_priority= nodes[i]->getBatchPriority()*2;
		}
		else if (prev_was_batched){
			cur_priority -= 1;
		}

		list[i].node= nodes[i];
		list[i].priority= cur_priority;

		prev_was_batched= nodes[i]->isBatched();
	}
}

void UpdateLine::append(UpdateLine other){
	PROFILE();
	list.append(std::move(other.list));
}

void UpdateLine::sort(){
	PROFILE();
	// Sorting should conform the following rules:
	// - nodes of the same group won't change order (stable sort)
	// - batched nodes will be in priority order
	// - batch of nodes will be contiguous
	std::stable_sort(list.begin(), list.end(),
			[] (const NodeInfo& a, const NodeInfo& b){
				return a.priority > b.priority;
			});
	
}

void UpdateLine::run(){
	PROFILE();
	if (NodeType::invalidCount > 0)
		return;

	for (const NodeInfo& info : list){
		try {
			info.node->baseUpdate();
		}
		catch (const global::Exception& e){
			print(debug::Ch::Nodes, debug::Vb::Critical,
					"Node error caught: %s. Do something!",
					info.node->identityDump().cStr());
		}
	}
}

} // nodes
} // clover
