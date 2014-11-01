#include "particletype.hpp"

namespace clover {
namespace visual {

ParticleVBOVertexData::ParticleVBOVertexData(){
	for (int32 i=0; i<2; i++){
		position[i]=
		velocity[i]=0;
		size[i]=0;
	}

	rotation=0;
	angularVelocity=0;

	for (int32 i=0; i<8; i++)
		params[i]=0;
}

util::DynArray<VertexAttribute> ParticleVBOVertexData::getAttributes(){
	util::DynArray<VertexAttribute> ret;

	// Position
	VertexAttribute a;
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= 0;
	a.name= "inPosition";
	ret.pushBack(a);

	// Velocity
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= 2*4;
	a.name= "inVelocity";
	ret.pushBack(a);

	// Size
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= 4*4;
	a.name= "inSize";
	ret.pushBack(a);

	// Rotation
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= 6*4;
	a.name= "inRotation";
	ret.pushBack(a);

	// AngularVelocity
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= 7*4;
	a.name= "inAngularVelocity";
	ret.pushBack(a);

	// Params
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 4;
	a.offset= 8*4;
	a.name= "inParams";
	ret.pushBack(a);

	// Params2
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 4;
	a.offset= 12*4;
	a.name= "inParams2";
	ret.pushBack(a);

	return ret;
}

ParticleSpawnData::ParticleSpawnData(){
}

} // visual
} // clover