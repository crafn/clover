#include "fluidparticle.hpp"
#include "util/mesh.hpp"

#include <stddef.h>

namespace clover {
namespace physics {

util::DynArray<visual::VertexAttribute> FluidParticle::getAttributes(){
	util::DynArray<visual::VertexAttribute> ret;
	visual::VertexAttribute a;

	a.offset= offsetof(FluidParticle, position);
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.name= "a_position";
	ret.pushBack(a);

	a.offset= offsetof(FluidParticle, velocity);
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.name= "a_velocity";
	ret.pushBack(a);
	
	a.offset= offsetof(FluidParticle, acceleration);
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.name= "a_acceleration";
	ret.pushBack(a);

	a.offset= offsetof(FluidParticle, radius);
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 1;
	a.name= "a_radius";
	ret.pushBack(a);

	a.offset= offsetof(FluidParticle, bias);
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 1;
	a.name= "a_bias";
	ret.pushBack(a);

	a.offset= offsetof(FluidParticle, density);
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 1;
	a.name= "a_density";
	ret.pushBack(a);

	a.offset= offsetof(FluidParticle, pressure);
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 1;
	a.name= "a_pressure";
	ret.pushBack(a);

	static_assert(offsetof(FluidParticle, chunkId) == 40, "Alignment error");
	a.offset= offsetof(FluidParticle, chunkId);
	a.elemType= hardware::GlState::Type::Uint8;
	a.elemCount= 1;
	a.name= "a_chunkId";
	ret.pushBack(a);

	return ret;
}

} // physics
} // clover