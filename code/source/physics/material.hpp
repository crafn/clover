#ifndef CLOVER_PHYSICS_MATERIAL_HPP
#define CLOVER_PHYSICS_MATERIAL_HPP

#include "build.hpp"
#include "script/reference.hpp"
#include "script/typestring.hpp"
#include "util/map.hpp"
#include "util/referencecountable.hpp"

namespace clover {
namespace physics {

class FixtureDef;
/// Use game::PhysicalMaterial instead of this for a fixture!
class Material : public util::ReferenceCountable, public script::NoCountReference {
public:
	Material();
	Material(real32 density, real32 friction, real32 restitution, real32 toughness= -1.0);
	virtual ~Material(){}

	void setDensity(real32 d);
	void setFriction(real32 f);
	void setRestitution(real32 r);
	void setToughness(real32 t);
	
	real32 getDensity() const { return density; }
	real32 getFriction() const { return friction; }
	real32 getRestitution() const { return restitution; }
	real32 getToughness() const { return toughness; }

private:
	void onChange();
	
	real32 density;
	real32 friction;
	real32 restitution;
	/// Minimum value of impulse field gradient which'll be enough to break
	/// Negative value == infinity
	real32 toughness;
};

} // physics
namespace util {

template <>
struct TypeStringTraits<physics::Material> {
	static util::Str8 type(){ return "physics::Material"; }
};

} // util
} // clover

#endif // CLOVER_PHYSICS_MATERIAL_HPP