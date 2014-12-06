#include "particlemanifold.hpp"
#include "util/time.hpp"

namespace clover {
namespace visual {

ParticleManifold::ParticleManifold():
		type(0),
		created(false),
		simulationKernel(0),
		spawnKernel(0),
		activeParticleOffset(0),
		activeParticleCount(0){


}

ParticleManifold::~ParticleManifold(){
}

void ParticleManifold::create(const ParticleType& type_){
	type= &type_;

	// Create buffer for OpenCL
	util::DynArray<ParticleVBOVertexData> data;
	data.resize(type->maxParticleCount);
	times.resize(type->maxParticleCount);
	for (auto& m : data){

	}
	for (auto& m : times){
		m= -100000;
	}

	mesh.setUsage(hardware::GlState::VaoUsage::DynamicCopy);
	mesh.setVertices(data);
	mesh.flush();

	mesh.setBoundingBox(util::BoundingBox<util::Vec2f>(util::Vec2f(0), util::Vec2f(10000000)));

	// Prepare the OpenCL program
	program.compile(type->programPath);
	simulationKernel= &program.createKernel("simulate");
	spawnKernel= &program.createKernel("spawn");

	ensure(mesh.getVao());
	particleBuffer.alias(hardware::ClState::ReadWrite, *mesh.getVao());

	program.attachToQueue(queue);

	// util::Set up material and drawing thingies
	ensure(type->material);
	model.setMaterial(*type->material);
	model.setMesh(mesh);

	modelEntityDef.setModel(model);
	modelEntityDef.setShadingType(visual::ModelEntityDef::Shading_Particle);
	//modelEntityDef.setLayer(visual::ModelEntityDef::Layer::Particles);

	created= true;
}

void ParticleManifold::spawn(util::DynArray<ParticleSpawnData>& particles){
	ensure(!particles.empty());
	ensure(created);

	// If spawning is too slow for many sources, spawn data could be pushed in only one buffer and submitted in update

	uint32 offset= activeParticleOffset+activeParticleCount;
	uint32 real_amount= particles.size();

	if (real_amount > type->maxParticleCount)
		real_amount= type->maxParticleCount;

	real64 cur_time= util::gGameClock->getTime();
	for (uint32 i=offset; i<offset + real_amount; ++i){
		times[i%type->maxParticleCount]= cur_time;
	}

	hardware::gGlState->finish();

	util::ParallelBuffer spawn_buf;
	spawn_buf.create(hardware::ClState::UseHostPtr, particles[0], particles.size());

	spawnKernel->setArgument(0, particleBuffer);
	spawnKernel->setArgument(1, spawn_buf);
	spawnKernel->setArgument(2, offset);
	spawnKernel->setArgument(3, real_amount);
	spawnKernel->setArgument(4, type->maxParticleCount);

	particleBuffer.acquire();

	util::ParallelKernel::Work w;
	w.globalSize= type->maxParticleCount;

	spawnKernel->enqueue(w);
	particleBuffer.release();
	queue.finish();

	//std::cout << "smokeID: " << currentSmokeID << "\n";

	activeParticleCount += real_amount;

	if (activeParticleCount >= type->maxParticleCount){
		activeParticleOffset += activeParticleCount - type->maxParticleCount;

		while (activeParticleOffset >= type->maxParticleCount)
			activeParticleOffset -= type->maxParticleCount;

		activeParticleCount= type->maxParticleCount;
	}
}

void ParticleManifold::update(){
	return; // Not in use currently
	ensure(created);

	real32 dt= util::gGameClock->getDeltaTime();

	hardware::gGlState->finish();

	simulationKernel->setArgument(0, particleBuffer);
	simulationKernel->setArgument(1, dt);

	particleBuffer.acquire();

	util::ParallelKernel::Work w;
	w.globalSize= type->maxParticleCount;
	simulationKernel->enqueue(w);

	particleBuffer.release();
	queue.finish();

	// Update active range
	real64 cur_time= util::gGameClock->getTime();

	while(activeParticleCount > 0){
		if (cur_time - times[activeParticleOffset] >= type->maxParticleLifeTime){
			++activeParticleOffset;
			--activeParticleCount;

			if (activeParticleOffset >= type->maxParticleCount)
				activeParticleOffset= 0;
		}
		else {
			break;
		}
	}
	mesh.setDrawRange(activeParticleOffset, activeParticleCount);
}

} // visual
} // clover