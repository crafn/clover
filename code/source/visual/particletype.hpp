#ifndef CLOVER_VISUAL_PARTICLETYPE_HPP
#define CLOVER_VISUAL_PARTICLETYPE_HPP

#include "build.hpp"
#include "visual/material.hpp"
#include "visual/vertexarrayobject.hpp"
#include "visual/mesh.hpp"
#include "util/parallel_program.hpp"
#include "util/parallel_queue.hpp"

namespace clover {
namespace visual {

/// @class ParticleVBOVertexData A 64-byte vertex representing a particle for gpu
struct ParticleVBOVertexData {
	ParticleVBOVertexData();

	real32 position[2];
	real32 velocity[2];
	real32 size[2];
	real32 rotation;
	real32 angularVelocity;
	real32 params[8];

	static util::DynArray<VertexAttribute> getAttributes();

} __attribute__((aligned(64)));

} // visual
namespace util {

template <>
struct MeshVertexTraits<visual::ParticleVBOVertexData> {
	typedef util::Vec2f BoundingBoxType;
	static util::Vec2f extractPosition(const visual::ParticleVBOVertexData& d){ return util::Vec2f {d.position[0], d.position[1]}; }

	static visual::ParticleVBOVertexData converted(const util::Vec2d& input){
		visual::ParticleVBOVertexData v;
		v.position[0]= input.x;
		v.position[1]= input.y;
		return v;
	}
};

template <>
struct Hash32<visual::ParticleVBOVertexData> {
	uint32 operator()(const visual::ParticleVBOVertexData& value) const {
		return rawArrayHash32(reinterpret_cast<const char*>(&value), sizeof(value));
	}
};

} // util
namespace visual {

/// @struct ParticleSpawnData Submitted to the util::ParallelProgram which spawns new particles
struct ParticleSpawnData {
	ParticleSpawnData();

	util::Vec2f position;
/*	util::Vec2f velocity;
	real32 rotation;
	real32 angularVelocity;

	real32 dummy[2];*/
}/* __attribute__((aligned(32)))*/;

/// @struct ParticleType Defines type of ParticleManifoldEntity
struct ParticleType {
	ParticleType():
		name("None"),
		material(0),
		maxParticleCount(256),
		maxParticleLifeTime(5.0),
		programPath("none"){}

	util::Str8 name;

	const visual::Material* material;

	uint32 maxParticleCount;
	real32 maxParticleLifeTime;

	// Replace with programName when programs are get from ResourceCache
	util::Str8 programPath;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_PARTICLETYPE_HPP