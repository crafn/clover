#include "global/env.hpp"
#include "particle_mgr.hpp"
#include "resources/cache.hpp"
#include "visual/entitylogic_particlesource.hpp"

namespace clover {
namespace visual {

ParticleMgr::ParticleMgr(){
	listenForEvent(global::Event::OnParticleSourceActivation);

	/// @todo Replace this by ResourceCache loading
	ParticleType& smoke_type= typeMap["Smoke"];
	smoke_type.name= "Smoke";
	smoke_type.programPath= "shaders/opencl/smoke.cl";
	smoke_type.material= &global::g_env->resCache->getResource<visual::Material>("smoke");
	manifoldMap["Smoke"].create(smoke_type);
}

void ParticleMgr::onEvent(global::Event& e){
	switch(e.getType()){
		case global::Event::OnParticleSourceActivation:

			// ParticleSourceEntity takes care of not sending same event multiple times
			if (e(global::Event::Active).getI())
				sources.pushBack( e(global::Event::Object).getPtr<ParticleSourceEntityLogic>() );
			else
				sources.erase(sources.find( e(global::Event::Object).getPtr<ParticleSourceEntityLogic>() ));

		break;

		default: break;
	}
}

void ParticleMgr::update(){
	for (auto& m : sources){
		//ensure(m->isActive());

		/// @todo Don't update or spawn if too far from the camera
		m->update();

		if (m->hasParticlesForSpawning()){

			auto it= manifoldMap.find(m->getParticleTypeName());

			ensure(it != manifoldMap.end());

			auto spawn_data= std::move(m->popSpawnData());
			it->second.spawn(spawn_data);

		}
	}

	for (auto& m : manifoldMap){
		m.second.update();
	}
}

ParticleManifold& ParticleMgr::getManifold(const util::Str8& name){
	auto it= manifoldMap.find(name);
	ensure(it != manifoldMap.end());
	return it->second;
}

} // visual
} // clover
