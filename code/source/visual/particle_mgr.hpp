#ifndef CLOVER_VISUAL_PARTICLE_MGR_HPP
#define CLOVER_VISUAL_PARTICLE_MGR_HPP

#include "build.hpp"
#include "visual/particlemanifold.hpp"
#include "visual/particletype.hpp"
#include "global/eventreceiver.hpp"
#include "util/hashmap.hpp"

namespace clover {
namespace visual {

class ParticleSourceEntityLogic;

/// @class ParticleMgr Manages particle manifolds, sources and types
class ParticleMgr : public global::EventReceiver {
public:
	ParticleMgr();

	virtual void onEvent(global::Event& e);

	void update();

	ParticleManifold& getManifold(const util::Str8& name);

private:
	util::DynArray<ParticleSourceEntityLogic*> sources;

	util::HashMap<util::Str8, ParticleType> typeMap;
	util::HashMap<util::Str8, ParticleManifold> manifoldMap;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_PARTICLE_MGR_HPP