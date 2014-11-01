#include "chunk_util.hpp"
#include "fluid_mgr.hpp"
#include "global/cfg_mgr.hpp"
#include "resources/cache.hpp"
#include "object_rigid.hpp"
#include "global/file.hpp"
#include "visual/camera.hpp"
// Proto
#include "gui/cursor.hpp"
#include "gui/gui_mgr.hpp"

namespace clover {
namespace physics {

static util::Vec2d cursorPos(){
	return gui::gGuiMgr->getCursor().getPosition().converted(util::Coord::World).
			getValue().casted<util::Vec2d>();
}

FluidMgr::FluidMgr(util::Vec2d gravity)
		: chunkSize(global::gCfgMgr->get("physics::fluidChunkSize", 16))
		, maxChunkCount(global::gCfgMgr->get("physics::maxFluidChunkCount", 8))
		, potFieldReso(
				global::gCfgMgr->get("physics::fluidPotentialFieldReso", 256))
		, distFieldReso(
				global::gCfgMgr->get("physics::fluidDistanceFieldReso", 256))
		, potField(potFieldReso, maxChunkCount)
		, distField(distFieldReso, maxChunkCount)
		, gravity(gravity.casted<util::Vec2f>())
		, spawnListener("host", "dev", "fluid_spawn", [this] (){
			const real32 particle_radius=
				global::gCfgMgr->get("physics::protoParticleRadius", 1.0);

			util::Vec2d rand= util::Vec2d{	util::Rand::continuous(-1.0, 1.0),
								util::Rand::continuous(-1.0, 1.0) }*0.7;
			util::Vec2d pos= cursorPos() + rand;
			FluidParticle p;
			p.position= asChunkOffset(pos, chunkSize);
			p.velocity= util::Vec2f{10.0, 0.0};
			p.radius= particle_radius;
			p.chunkId= getChunkId( chunkVec(pos, chunkSize) );

			util::DynArray<FluidParticle> data;
			data.pushBack(p);
			spawn(data);
		})
		, spawn2Listener("host", "dev", "fluid_spawn2", [this] (){
			const real32 particle_radius=
				global::gCfgMgr->get("physics::protoParticleRadius", 1.0);

			util::DynArray<FluidParticle> data;
			for (SizeType i= 0; i < 100; ++i){
				util::Vec2d rand= util::Vec2d{	util::Rand::continuous(-1.0, 1.0),
									util::Rand::continuous(-1.0, 1.0) }*2.5;
				util::Vec2d pos= cursorPos() + rand;
				FluidParticle p;
				p.position= asChunkOffset(pos, chunkSize);
				p.velocity= util::Vec2f{0.0, 0.0};
				p.radius= particle_radius;
				p.chunkId= getChunkId( chunkVec(pos, chunkSize) );
				data.pushBack(p);
			}

			spawn(data);
		}){

	ensure_msg(maxChunkCount < chunkCountHardLimit,
			"physics::maxFluidChunkCount == %i >= %i",
			maxChunkCount, (int32)chunkCountHardLimit);

	// Simulation
	{
		const util::Str8 prog_path= "fluids/simulation.cl";

		// Prepare the OpenCL program
		program.compile(prog_path);

		simulationKernel= &program.createKernel("simulate");
		spawnKernel= &program.createKernel("spawn");
		program.attachToQueue(queue);

		chunks.resize(maxChunkCount);

		chunkInfoTex= hardware::gGlState->genTex();
		hardware::gGlState->bindTex(hardware::GlState::TexTarget::Tex2d, chunkInfoTex);
		hardware::gGlState->setTexParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		hardware::gGlState->setTexParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	// Particles
	{
		const SizeType max_particle_count=
			global::gCfgMgr->get("physics::protoParticleCount", 100);
		const real32 particle_radius=
			global::gCfgMgr->get("physics::protoParticleRadius", 1.0);

		util::DynArray<FluidParticle> data;
		data.resize(max_particle_count);
		for (auto& p : data){
			p.chunkId= chunkIdNone;
		}

		particles.setUsage(hardware::GlState::VaoUsage::StaticDraw);
		particles.setVertices(data);
		particles.flush();
		particles.setBoundingBox(util::BoundingBox<util::Vec2f>(util::Vec2f(0), util::Vec2f(10000000)));

		activeParticleOffset= 0;
		activeParticleCount= 0;
		particleBuffer.alias(hardware::ClState::ReadWrite, *particles.getVao());

		particleForces.resize(max_particle_count);
		particleForceBuffer.create(
				hardware::ClState::UseHostPtr,
				particleForces.front(),
				particleForces.size());
		particleForceBuffer.attachToQueue(queue);
	}

	potFieldBuf.alias(hardware::ClState::ReadOnly, potField.getFbo());
	distFieldBuf.alias(hardware::ClState::ReadOnly, distField.getFbo());
}

FluidMgr::~FluidMgr(){
	hardware::gGlState->deleteTex(chunkInfoTex);
}

void FluidMgr::preUpdate(){
	PROFILE();

	updateChunkInfoTex();
	potField.update(chunkSize, particles, chunkInfoTex);
	distField.update(chunkSize, chunkPosToId, chunkInfoTex);
	hardware::gGlState->flush();
}

void FluidMgr::update(real32 dt){
	{
		PROFILE();
		hardware::gGlState->finish();
	}

	{ PROFILE();
		chunkInfoTexBuffer.aliasTex(
				hardware::ClState::ReadOnly,
				(uint32)hardware::GlState::TexTarget::Tex2d,
				chunkInfoTex);

		simulationKernel->setArgument(0, particleBuffer);
		simulationKernel->setArgument(1, particleForceBuffer);
		simulationKernel->setArgument(2, chunkSize);
		simulationKernel->setArgument(3, maxChunkCount);
		simulationKernel->setArgument(4, chunkInfoTexBuffer);
		simulationKernel->setArgument(5, potFieldBuf);
		simulationKernel->setArgument(6, potFieldReso);
		simulationKernel->setArgument(7, distFieldBuf);
		simulationKernel->setArgument(8, distFieldReso);
		simulationKernel->setArgument(9, dt);
		simulationKernel->setArgument(10, gravity);

		particleBuffer.acquire();
		potFieldBuf.acquire();
		distFieldBuf.acquire();
		chunkInfoTexBuffer.acquire();

		util::ParallelKernel::Work w;
		w.globalSize= particles.getVertexCount();
		simulationKernel->enqueue(w);

		chunkInfoTexBuffer.release();
		distFieldBuf.release();
		potFieldBuf.release();
		particleBuffer.release();

		queue.flush();
	}
}

void FluidMgr::postUpdate(){
	{ PROFILE();
		queue.finish();
	}
	{ PROFILE();
		particleForceBuffer.read();
	}
	
	{ PROFILE();
		chunkInfoTexBuffer.reset();
	}

	{ PROFILE();

		// Apply forces to bodies
		/// @todo	Ignore force if body was created on this frame,
		///			because then object has been replaced

		const auto& pool= RigidObject::poolView();
		for (const ParticleForce& pforce : particleForces){
			if (	pforce.objectId == objectIdNone || 
					pforce.chunkId == chunkIdNone)
				continue;

			ensure(pforce.objectId < pool.size())
			RigidObject* obj= pool[pforce.objectId];

			if (!obj)
				continue;

			/// @todo Sum forces before applying
			util::Vec2i chunk_pos= getChunkPos(pforce.chunkId);

			util::Vec2d force= pforce.force.casted<util::Vec2d>();
			util::Vec2d worldpoint= worldVec(pforce.point, chunk_pos, chunkSize);
			obj->applyForce(force, worldpoint);
		}
	}
}

void FluidMgr::addChunk(util::Vec2i pos){
	ensure(!util::contains(chunkPosToId, pos));

	if (chunkPosToId.size() >= maxChunkCount){
		print(debug::Ch::Phys, debug::Vb::Moderate, "Too many fluid chunks");
		return;
	}

	ensure(chunkPosToId.find(pos) == chunkPosToId.end());

	for (SizeType id= 0; id < maxChunkCount; ++id){
		if (!chunks[id].active){
			chunks[id].position= pos;
			chunks[id].active= true;
			chunkPosToId[pos]= id;
			return;
		}
	}

	release_ensure_msg(0, "Adding fluid chunk failed");
}

void FluidMgr::removeChunk(util::Vec2i pos){
	auto it= chunkPosToId.find(pos);
	if (it == chunkPosToId.end())
		return;

	uint8 remove_id= it->second;
	chunkPosToId.erase(it);
}

void FluidMgr::spawn(util::DynArray<FluidParticle>& spawndata){
	PROFILE();
	if (spawndata.empty())
		return;

	uint32 max_count= particles.getVertexCount();

	uint32 offset= activeParticleOffset + activeParticleCount;
	uint32 real_amount= spawndata.size();

	if (real_amount > max_count)
		real_amount= max_count;

	hardware::gGlState->finish();

	util::ParallelBuffer spawn_buf;
	spawn_buf.create(hardware::ClState::UseHostPtr, spawndata[0], spawndata.size());

	spawnKernel->setArgument(0, particleBuffer);
	spawnKernel->setArgument(1, spawn_buf);
	spawnKernel->setArgument(2, offset);
	spawnKernel->setArgument(3, real_amount);
	spawnKernel->setArgument(4, max_count);

	particleBuffer.acquire();
	{
		PROFILE();
		util::ParallelKernel::Work w;
		w.globalSize= max_count;
		spawnKernel->enqueue(w);
	}
	particleBuffer.release();

	/// @todo Move to more optimal location
	queue.finish();

	activeParticleCount += real_amount;

	if (activeParticleCount >= max_count){
		activeParticleOffset += activeParticleCount - max_count;

		activeParticleOffset += 1; // TEMP

		while (activeParticleOffset >= max_count)
			activeParticleOffset -= max_count;

		activeParticleCount= max_count;
	}

	particles.setDrawRange(activeParticleOffset, activeParticleCount);
}

void FluidMgr::updateChunkInfoTex(){
	// Format of 'data'
	// For every chunk
	// - chunk position (int32 + int32, 1 texel)
	// - adjacent ids (8*uint8, 1 texel)
	util::DynArray<uint16> data;
	data.resize(chunks.size()*2*4);

	SizeType data_i= 0;
	for (int32 id= 0; id < (int32)chunks.size(); ++id){
		util::Vec2i pos= chunks[id].position;
		uint32 x= (uint32)pos.x;
		uint32 y= (uint32)pos.y;
		/// @todo Check how endianness works with glsl/opengl
		data[data_i + 0]= (uint16)((x & 0x0000FFFF) >> 0);
		data[data_i + 1]= (uint16)((x & 0xFFFF0000) >> 16);
		data[data_i + 2]= (uint16)((y & 0x0000FFFF) >> 0);
		data[data_i + 3]= (uint16)((y & 0xFFFF0000) >> 16);
		data_i += 4;

		std::array<uint8, 8> adjacent= getAdjacentChunks(id);
		for (SizeType i= 0; i < adjacent.size(); i += 2){
			data[data_i]= adjacent[i] + (adjacent[i + 1] << 8);
			++data_i;
		}
	}

	ensure(data_i == data.size());

	hardware::gGlState->bindTex(hardware::GlState::TexTarget::Tex2d, chunkInfoTex);
	hardware::gGlState->submitTexData(
			hardware::GlState::TexFormat::Rgba16,
			hardware::GlState::Type::Uint16,
			util::Vec2i{2, (int32)chunks.size()},
			data.data());
}

std::array<uint8, 8> FluidMgr::getAdjacentChunks(uint8 id) const {
	// Order should match with shaders and simulation
	std::array<util::Vec2i, 8> offsets= {
		util::Vec2i{1,  0},
		util::Vec2i{1,  1},
		util::Vec2i{0,  1},
		util::Vec2i{-1, 1},
		util::Vec2i{-1, 0},
		util::Vec2i{-1, -1},
		util::Vec2i{0,  -1},
		util::Vec2i{1,  -1}
	};
	
	auto inactive_adjacents= [&offsets] (){
		std::array<uint8, 8> adjacents;
		for (auto& v : adjacents)
		   v= chunkIdNone;	
		return adjacents;
	};

	std::array<uint8, 8> adjacents;

	if (id == chunkIdNone)
		return inactive_adjacents();

	util::Vec2i this_pos= getChunkPos(id);
	if (!isActiveChunk(this_pos))
		return inactive_adjacents();

	for (SizeType i= 0; i < adjacents.size(); ++i){
		adjacents[i]= getChunkId(this_pos + offsets[i]);
	}
	
	return adjacents;
}

uint8 FluidMgr::getChunkId(util::Vec2i pos) const {
	auto it= chunkPosToId.find(pos);
	if (it == chunkPosToId.end())
		return chunkIdNone;
	else
		return it->second;
}

util::Vec2i FluidMgr::getChunkPos(uint8 id) const {
	ensure(id != chunkIdNone);
	ensure(id < chunks.size());
	ensure(chunks.size() == maxChunkCount);
	return chunks[id].position;
}

bool FluidMgr::isActiveChunk(util::Vec2i pos) const {
	auto it= chunkPosToId.find(pos);
	if (it == chunkPosToId.end()){
		return false;
	}
	else {
		ensure(it->second < chunks.size());
		ensure(chunks[it->second].active);
		return true;
	}
}

} // physics
} // clover
