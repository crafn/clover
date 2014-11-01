#ifndef CLOVER_PHYSICS_FLUID_PARTICLE_HPP
#define CLOVER_PHYSICS_FLUID_PARTICLE_HPP

#include "build.hpp"
#include "util/dyn_array.hpp"
#include "util/mesh.hpp"
#include "util/vector.hpp"
#include "visual/vertexattribute.hpp"

namespace clover {
namespace physics {

struct FluidParticle {
	/// Position measured from chunk corner
	util::Vec2f position;
	util::Vec2f velocity;
	util::Vec2f acceleration;
	real32 radius= 1.0;
	/// Radius bias due to pressure
	real32 bias= 1.0;

	// 32 byte mark

	real32 density= 100.0;
	real32 pressure= 0.0;

	uint8 chunkId= 0;
	uint8 padding[7]= {0};

	// 48 byte mark

	static util::DynArray<visual::VertexAttribute> getAttributes();
};

} // physics
namespace util {

template <>
struct MeshVertexTraits<physics::FluidParticle> {
	typedef util::Vec2f BoundingBoxType;

	static util::Vec2f extractPosition(const physics::FluidParticle& p){
		return p.position;
	}

	static physics::FluidParticle converted(const util::Vec2d& input){
		physics::FluidParticle p;
		p.position= input.converted<util::Vec2f>();
		return p;
	}
};

template <>
struct Hash32<physics::FluidParticle> {
	uint32 operator()(const physics::FluidParticle& p) const {
		return rawArrayHash32(reinterpret_cast<const char*>(&p), sizeof(p));
	}
};

} // util
} // clover

#endif // CLOVER_PHYSICS_FLUID_PARTICLE_HPP