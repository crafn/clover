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
	
/// Data of one vertex for TriMesh and to be uploaded to the gpu
struct Vertex {
	Vertex();

	util::Vec2f position;
	util::Vec2f padding;
	util::Vec2f uv;
	util::Color color;
	util::Vec2f tangent;

	// inParams[0]: 1-kerroin ambient-valaistukseen
	// inParams[1]: curven leveys
	// inParams[2]: curven uv-leveys
	real32 params[4];

	static util::DynArray<VertexAttribute> getAttributes();
} __attribute__((aligned(64)));

} // visual
namespace util {

template <>
struct MeshVertexTraits<visual::Vertex> {
	typedef util::Vec2f BoundingBoxType;
	static util::Vec2f extractPosition(const visual::Vertex& d){ return d.position; }

	static visual::Vertex converted(const util::Vec2d& input){
		visual::Vertex v;
		v.position= input.casted<util::Vec2f>();
		return v;
	}
};

template <>
struct ObjectNodeTraits<visual::Vertex> {
	using Value= visual::Vertex;
	static util::ObjectNode serialized(const Value& value){
		util::ObjectNode ob;
		ob["position"].setValue(value.position);
		ob["uv"].setValue(value.uv);
		return (ob);
	}
	
	static Value deserialized(const util::ObjectNode& ob){
		Value v;
		v.position= ob.get("position").getValue<util::Vec2f>();
		v.uv= ob.get("uv").getValue<util::Vec2f>();
		return (v);
	}
};

template <>
struct Hash32<visual::Vertex> {
	uint32 operator()(const visual::Vertex& value) const {
		return rawArrayHash32(reinterpret_cast<const char*>(&value), sizeof(value));
	}
};

} // util
} // clover

#endif // CLOVER_VISUAL_VERTEX_HPP