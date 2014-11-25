#include "vertex.hpp"
#include <cstddef>

namespace clover {
namespace visual {

util::DynArray<VertexAttribute> Vertex::getAttributes(){
	util::DynArray<VertexAttribute> ret;

	// Position
	VertexAttribute a;
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= offsetof(Vertex, position);
	a.name= "a_position";
	ret.pushBack(a);

	// Uv0
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= offsetof(Vertex, uv);
	a.name= "a_uv";
	ret.pushBack(a);

	// util::Color
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 4;
	a.offset= offsetof(Vertex, color);
	a.name= "a_color";
	ret.pushBack(a);

	// Tangent
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 2;
	a.offset= offsetof(Vertex, tangent);
	a.name= "a_tangent";
	ret.pushBack(a);

	return ret;
}

} // visual
} // clover
