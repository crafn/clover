#ifndef CLOVER_VISUAL_VERTEX_HPP
#define CLOVER_VISUAL_VERTEX_HPP

#include "build.hpp"
#include "util/color.hpp"
#include "util/hash.hpp"
#include "util/mesh.hpp"
#include "util/objectnode.hpp"
#include "util/vector.hpp"
#include "vertexattribute.hpp"

namespace clover {
namespace visual {

struct Vertex {
	util::Vec3f position;
	util::Vec2f uv;
	util::Color color;
	util::Vec3f tangent;
	real32 pad[4]= {0};

	static util::DynArray<VertexAttribute> getAttributes();
} __attribute__((aligned(64)));

} // visual
namespace util {

template <>
struct MeshVertexTraits<visual::Vertex> {
	typedef util::Vec3f BoundingBoxType;
	static util::Vec3f extractPosition(const visual::Vertex& d)
	{ return d.position; }

	static visual::Vertex converted(const util::Vec2d& input)
	{ return converted(input.casted<util::Vec3d>()); }

	static visual::Vertex converted(const util::Vec3d& input)
	{
		visual::Vertex v;
		v.position= input.casted<util::Vec3f>();
		return v;
	}
};

template <>
struct ObjectNodeTraits<visual::Vertex> {
	using Value= visual::Vertex;
	static util::ObjectNode serialized(const Value& value)
	{
		util::ObjectNode ob;
		ob["position"].setValue(value.position);
		ob["uv"].setValue(value.uv);
		return (ob);
	}

	static Value deserialized(const util::ObjectNode& ob)
	{
		Value v;
		v.position= ob.get("position").getValue<util::Vec3f>();
		v.uv= ob.get("uv").getValue<util::Vec2f>();
		return (v);
	}
};

template <>
struct Hash32<visual::Vertex> {
	uint32 operator()(const visual::Vertex& value) const
	{
		return rawArrayHash32(reinterpret_cast<const char*>(&value), sizeof(value));
	}
};

} // util
} // clover

#endif // CLOVER_VISUAL_VERTEX_HPP
