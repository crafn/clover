#ifndef CLOVER_VISUAL_PARTICLEMANIFOLD_HPP
#define CLOVER_VISUAL_PARTICLEMANIFOLD_HPP

#include "build.hpp"
#include "visual/particletype.hpp"
#include "visual/mesh.hpp"
#include "visual/model.hpp"
#include "visual/entity.hpp"
#include "visual/entity_def_model.hpp"
#include "util/parallel_program.hpp"
#include "util/parallel_buffer.hpp"

namespace clover {
namespace util {

class ParallelKernel;

} // utils
namespace visual {

struct ParticleSpawnData;

/// @class ParticleManifoldEntity Contains all particles of one type
class ParticleManifold {
public:
	ParticleManifold();
	ParticleManifold(ParticleManifold&&)= default; // Don't call after creation
	ParticleManifold(const ParticleManifold&)= delete;
	virtual ~ParticleManifold();

	ParticleManifold& operator=(ParticleManifold&&)= default;

	void create(const ParticleType& type);
	void spawn(util::DynArray<ParticleSpawnData>& spawndata);
	void update();

private:
	const ParticleType* type;

	bool created;

	util::ParallelProgram program;
	util::ParallelQueue queue;

	util::ParallelKernel* simulationKernel;
	util::ParallelKernel* spawnKernel;

	util::ParallelBuffer particleBuffer;

	util::DynArray<real64> times;
	PointMesh<ParticleVBOVertexData> mesh;

	Model model;
	ModelEntityDef modelEntityDef;
	Entity modelEntity;
	
	uint32 activeParticleOffset;
	uint32 activeParticleCount;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_PARTICLEMANIFOLD_HPP