#ifndef CLOVER_PHYSICS_FLUID_MGR_HPP
#define CLOVER_PHYSICS_FLUID_MGR_HPP

#include "build.hpp"
#include "distancefield.hpp"
#include "fluidparticle.hpp"
#include "potentialfield.hpp"
#include "util/parallel_program.hpp"
#include "util/parallel_queue.hpp"
#include "util/parallel_buffer.hpp"
#include "visual/framebuffer.hpp"
#include "visual/shader.hpp"
// Proto
#include "ui/hid/actionlistener.hpp"
#include "visual/mesh.hpp"
#include "visual/model.hpp"
#include "visual/entity.hpp"
#include "visual/entity_def_model.hpp"

namespace clover {
namespace util {

class ParallelKernel;

} // util
namespace physics {

/// Fluid proto
class FluidMgr {
public:
	FluidMgr(util::Vec2d gravity);
	~FluidMgr();

	/// Starts async gl update
	void preUpdate();
	/// Starts async cl simulation (waits gl)
	void update(real32 dt);
	/// Waits for simulation to finish
	/// @note Must be called before rendering particles
	void postUpdate();

	void addChunk(util::Vec2i pos);
	void removeChunk(util::Vec2i pos);
	uint32 getChunkSize() const { return chunkSize; }
	uint32 getMaxChunkCount() const { return maxChunkCount; }

	hardware::GlState::TexDId getChunkInfoTexDId() const { return chunkInfoTex; }

	const visual::PointMesh<FluidParticle>& getParticles() const { return particles; }

	const DistanceField& getDistanceField() const { return distField; }

private:
	void spawn(util::DynArray<FluidParticle>& particles);

	void updateChunkInfoTex();
	std::array<uint8, 8> getAdjacentChunks(uint8 id) const;
	uint8 getChunkId(util::Vec2i pos) const;
	util::Vec2i getChunkPos(uint8 id) const;
	bool isActiveChunk(util::Vec2i pos) const;

	// Limited by byte size
	static constexpr int32 chunkCountHardLimit= 256;
	static constexpr uint8 chunkIdNone= 255; // Last
	static constexpr uint16 objectIdNone= (uint16)-1;

	struct Chunk {
		util::Vec2i position;
		bool active= false;
	};

	// Force that particle applies to physics::RigidObject
	struct ParticleForce {
		util::Vec2f point;
		util::Vec2f force;
		uint16 objectId= objectIdNone;
		uint8 chunkId= chunkIdNone;
		uint8 padding[3];
	};

	// Simulation
	util::ParallelProgram program;
	util::ParallelQueue queue;
	util::ParallelKernel* simulationKernel= nullptr;
	util::ParallelKernel* spawnKernel= nullptr;
	uint32 chunkSize;
	uint32 maxChunkCount;
	uint32 potFieldReso;
	uint32 distFieldReso;
	// Defines ids of chunks as their indexes
	// Always size of maxChunkCount
	util::DynArray<Chunk> chunks;
	// Maps active chunk positions to their ids
	util::Map<util::Vec2i, uint8> chunkPosToId;
	// Texture that contains neighbor & position info for every chunk id
	hardware::GlState::TexDId chunkInfoTex;
	util::ParallelBuffer chunkInfoTexBuffer;

	// Particles
	visual::PointMesh<FluidParticle> particles;
	util::ParallelBuffer particleBuffer;
	uint32 activeParticleOffset;
	uint32 activeParticleCount;
	util::DynArray<ParticleForce> particleForces;
	util::ParallelBuffer particleForceBuffer;

	// Potential field of particles
	PotentialField potField;
	util::ParallelBuffer potFieldBuf;

	// Distance field of solid objects
	DistanceField distField;
	util::ParallelBuffer distFieldBuf;

	util::Vec2f gravity;

	ui::hid::ActionListener<> spawnListener;
	ui::hid::ActionListener<> spawn2Listener;
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_FLUID_MGR_HPP