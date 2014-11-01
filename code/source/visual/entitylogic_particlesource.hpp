#ifndef CLOVER_VISUAL_ENTITYLOGIC_PARTICLESOURCE_HPP
#define CLOVER_VISUAL_ENTITYLOGIC_PARTICLESOURCE_HPP

#include "build.hpp"
#include "util/dyn_array.hpp"
#include "util/transform.hpp"
#include "visual/entitylogic.hpp"
#include "visual/entity_def.hpp"
#include "visual/particletype.hpp"

namespace clover {
namespace visual {

class ParticleManifold;

class ParticleSourceEntityLogic : public EntityLogic {
public:
	ParticleSourceEntityLogic(const EntityDef& def);
	virtual ~ParticleSourceEntityLogic();

	void setParticleType(const util::Str8& type){ manifoldName= type; }

	virtual void update();

	bool hasParticlesForSpawning() const { return !spawnData.empty(); }
	util::DynArray<ParticleSpawnData> popSpawnData();

	const util::Str8& getParticleTypeName() const { return manifoldName; }

protected:
	void pushSpawnData(const ParticleSpawnData& d){ spawnData.pushBack(d); }

private:
	util::Str8 manifoldName;
	util::DynArray<ParticleSpawnData> spawnData;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_ENTITYLOGIC_PARTICLESOURCE_HPP