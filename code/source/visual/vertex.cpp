#include "vertex.hpp"

namespace clover {
namespace visual {

Vertex::Vertex(){
	for(int32 i=0; i<4; i++){
		params[i]=0;
	}
}

util::DynArray<VertexAttribute> Vertex::getAttributes(){
	util::DynArray<VertexAttribute> ret;

	// Position
	VertexAttribute a;
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= 0;
	a.name= "inPosition";
	ret.pushBack(a);

	// Normal
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= 2*4;
	a.name= "inNormal";
	ret.pushBack(a);

	// Uv0
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= 4*4;
	a.name= "inTexCoord";
	ret.pushBack(a);

	// util::Color
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 4;
	a.offset= 6*4;
	a.name= "inColor";
	ret.pushBack(a);

	// Tangent
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= 10*4;
	a.name= "inTangent";
	ret.pushBack(a);

	// Params
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 4;
	a.offset= 12*4;
	a.name= "inParams";
	ret.pushBack(a);

	return ret;
}

} // visual
} // clover