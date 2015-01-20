#include "util/objectnode.hpp"
#include "vertex.hpp"

#include <cstddef>

namespace clover {
namespace visual {

util::DynArray<VertexAttribute> Vertex::getAttributes()
{
	util::DynArray<VertexAttribute> ret;

	// Position
	VertexAttribute a;
	a.elemType= hardware::GlState::Type::Real32;
	a.elemCount= 3;
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
	a.elemCount= 3;
	a.offset= offsetof(Vertex, tangent);
	a.name= "a_tangent";
	ret.pushBack(a);

	return ret;
}

} // visual
namespace util {

util::ObjectNode ObjectNodeTraits<visual::Vertex>::
serialized(const Value& value)
{
	util::ObjectNode ob;
	ob["position"].setValue(value.position);
	ob["uv"].setValue(value.uv);
	return (ob);
}

auto ObjectNodeTraits<visual::Vertex>::
deserialized(const util::ObjectNode& ob) -> Value
{
	Value v;
	v.position= ob.get("position").getValue<util::Vec3f>();
	v.uv= ob.get("uv").getValue<util::Vec2f>();
	return (v);
}

} // util
} // clover
