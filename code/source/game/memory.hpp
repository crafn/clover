#ifndef CLOVER_GAME_MEMORY_HPP
#define CLOVER_GAME_MEMORY_HPP

/// @todo Move to global!

#include "build.hpp"
#include "util/mem_pool_linear.hpp"
#include "util/ator_bound.hpp"

namespace clover {
namespace game {

struct SingleFrameStorageTag;
using SingleFrameStorage=
	util::BoundAtorStorage<	util::LinearMemPool,
								SingleFrameStorageTag>;

/// @usage util::DynArray<int, game::SingleFrameAtor> array;
template <typename T>
using SingleFrameAtor= util::BoundAtor<T, SingleFrameStorage>;

void createMemoryPools(SizeType single_frame_mem_size);
void destroyMemoryPools();

} // game
} // clover

#endif // CLOVER_GAME_MEMORY_HPP
