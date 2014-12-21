#ifndef CLOVER_PHYSICS_DISTANCEFIELD_HPP
#define CLOVER_PHYSICS_DISTANCEFIELD_HPP

#include "build.hpp"
#include "collision/baseshape_polygon.hpp"
#include "global/memory.hpp"
#include "util/dyn_array.hpp"
#include "util/map.hpp"
#include "util/mesh.hpp"
#include "util/vector.hpp"
#include "visual/framebuffer.hpp"
#include "visual/mesh.hpp"
#include "visual/shader.hpp"
#include "visual/vertexarrayobject.hpp"
#include "visual/vertexattribute.hpp"

namespace clover {
namespace physics {

/// Represents a convex box2d polygon
struct PolyVertex {
	util::Vec2f position;
	util::Vec2f velocity;
	real32 rotation= 0.0;
	real32 angularVelocity= 0.0;

	static constexpr SizeType maxVertexCount=
		collision::PolygonBaseShape::maxInternalVertexCount;

	uint16 id= 0;
	uint8 chunkId= 0;
	uint8 vertexCount= 0;
	util::Vec2f vertices[maxVertexCount];

	static util::DynArray<visual::VertexAttribute> getAttributes();
};

static_assert(PolyVertex::maxVertexCount == 8, "Shader is configured for 8");
static_assert(
		PolyVertex::maxVertexCount <
			std::numeric_limits<decltype(PolyVertex::vertexCount)>::max(),
		"Too small type for vertex count");
static_assert(
		std::is_standard_layout<PolyVertex>::value == true,
		"Layout error");

}

namespace util {

template <>
struct MeshVertexTraits<physics::PolyVertex> {
typedef util::Vec2f BoundingBoxType;

static util::Vec2f extractPosition(const physics::PolyVertex& p){
	return p.position;
}

static physics::PolyVertex converted(const util::Vec2d& input){
	physics::PolyVertex p;
	p.position= input.converted<util::Vec2f>();
	return p;
}
};

} // util

namespace physics {

/// Intermediate vertex format in distance field calculation
struct DistFieldVertex {
	util::Vec2f position;
	util::Vec2f velocity;
	util::Vec2f uv;
	/// @todo Make transform feedback work with smaller datatypes
	uint32 id;
	uint32 chunkId;

	static util::DynArray<visual::VertexAttribute> getAttributes();
};

/// Vertex used to draw slices of distance field
struct VolumVertex {
	util::Vec2f position;
	util::Vec3f uv;
	real32 padding;

	static util::DynArray<visual::VertexAttribute> getAttributes();
};

/// Distance field of solid objects
/// Used in fluid simulation
/// @note Uses OpenGL for calculations
class DistanceField {
public:
	using PosToIdMap= util::Map<util::Vec2i, uint8>;

	DistanceField(uint32 chunk_reso, uint32 chunks);
	void update(uint32 chunk_size,
				const PosToIdMap& pos_to_id,
				hardware::GlState::TexDId adj_chunks_tex);

	const visual::Framebuffer& getFbo() const { return fbo; }

	const visual::VertexArrayObject<VolumVertex>&
	getMappedMesh() const { return mappedFieldMesh; }

private:
	struct PolyGenResult {
		util::DynArray<PolyVertex, global::SingleFrameAtor> polys;
		SizeType distMeshTriCount;
	};

	PolyGenResult genPolys(	uint32 chunk_size,
							const PosToIdMap& pos_to_id) const;

	// Produces triangle mesh from PolyVertices
	visual::Shader meshShader;
	visual::VertexArrayObject<DistFieldVertex> mesh;

	// Draws triangle mesh properly to 3d distance field
	visual::Shader drawShader;
	visual::Framebuffer fbo;
	uint32 chunkCount;

	// Maps distance field to world as a mesh
	visual::VertexArrayObject<VolumVertex> mappedFieldMesh;
};

} // physics
namespace util {

template <>
struct Hash32<physics::PolyVertex> {
	uint32 operator()(const physics::PolyVertex& p) const {
		return rawArrayHash32(reinterpret_cast<const char*>(&p), sizeof(p));
	}
};

} // util
} // clover

#endif // CLOVER_PHYSICS_DISTANCEFIELD_HPP
