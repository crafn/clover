#include "entitylogic_particlesource.hpp"
#include "global/event.hpp"

namespace clover {
namespace visual {

ParticleSourceEntityLogic::ParticleSourceEntityLogic(const EntityDef& def)
		: EntityLogic(def){
}

ParticleSourceEntityLogic::~ParticleSourceEntityLogic(){
}

void ParticleSourceEntityLogic::update(){
}

util::DynArray<ParticleSpawnData> ParticleSourceEntityLogic::popSpawnData(){
	util::DynArray<ParticleSpawnData> ret= std::move(spawnData);
	spawnData.clear();
	return (ret);
}

} // visual
} // clover