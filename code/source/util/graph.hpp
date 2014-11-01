#ifndef CLOVER_UTIL_GRAPH_HPP
#define CLOVER_UTIL_GRAPH_HPP

#include "build.hpp"
#include "util/containeralgorithms.hpp"
#include "util/set.hpp"

namespace clover {
namespace util {

template <typename C, typename T>
C findGraphNodes(
		C starting_nodes,
		std::function<C (T)> get_connected){
	C found;

	std::function<void (C)> recursion= [&] (C next){
		for (const auto& node : next){
			if (!contains(found, node)){
				fastInsert(found, node);
				recursion(get_connected(node));
			}
		}
	};

	recursion(std::move(starting_nodes));

	return found;
}

} // util
} // clover

#endif // CLOVER_UTIL_GRAPH_HPP