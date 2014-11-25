#include "particletype.hpp"
#include <cstddef>

namespace clover {
namespace visual {

ParticleVBOVertexData::ParticleVBOVertexData(){
	for (int32 i= 0; i < 2; ++i) {
		position[i]= 0;
		velocity[i]= 0;
		size[i]= 0;
	}

	rotation= 0;
	angularVelocity= 0;

	for (int32 i= 0; i < 4; ++i)
		color[i]= 0;
}

util::DynArray<VertexAttribute> ParticleVBOVertexData::getAttributes(){
	util::DynArray<VertexAttribute> ret;

	// position
	VertexAttribute a;
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= offsetof(ParticleVBOVertexData, position);
	a.name= "a_position";
	ret.pushBack(a);

	// velocity
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= offsetof(ParticleVBOVertexData, velocity);
	a.name= "a_velocity";
	ret.pushBack(a);

	// size
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= offsetof(ParticleVBOVertexData, size);
	a.name= "a_size";
	ret.pushBack(a);

	// rotation
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= offsetof(ParticleVBOVertexData, rotation);
	a.name= "a_rotation";
	ret.pushBack(a);

	// angularVelocity
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= offsetof(ParticleVBOVertexData, angularVelocity);
	a.name= "a_angularVelocity";
	ret.pushBack(a);

	// color
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 4;
	a.offset= offsetof(ParticleVBOVertexData, color);
	a.name= "a_color";
	ret.pushBack(a);

	return ret;
}

ParticleSpawnData::ParticleSpawnData(){
}

} // visual
} // clover
