#ifndef CLOVER_PHYSICS_CHUNK_UTIL_HPP
#define CLOVER_PHYSICS_CHUNK_UTIL_HPP

#include "build.hpp"
#include "util/vector.hpp"

namespace clover {
namespace physics {

inline
util::Vec2i chunkVec(util::Vec2d world_pos, real64 chunk_size)
{ return (world_pos/chunk_size).discretized<int32>(); }

inline
util::Vec2f asChunkOffset(util::Vec2d world_pos, real64 chunk_size)
{
	util::Vec2d chunk_pos= (chunkVec(world_pos, chunk_size)*chunk_size).
						casted<util::Vec2d>();
	return	(world_pos - chunk_pos).casted<util::Vec2f>();
}

inline
util::Vec2d worldVec(util::Vec2f chunk_offset, util::Vec2i chunk_pos, real64 chunk_size)
{ return chunk_offset.casted<util::Vec2d>() + chunk_pos.casted<util::Vec2d>()*chunk_size; }

} // physics
} // clover

#endif // CLOVER_PHYSICS_CHUNK_UTIL_HPP
