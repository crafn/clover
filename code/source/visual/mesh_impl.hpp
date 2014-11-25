#ifndef CLOVER_VISUAL_MESH_IMPL_HPP
#define CLOVER_VISUAL_MESH_IMPL_HPP

#include "build.hpp"
#include "vertexarrayobject.hpp"
#include "util/transform.hpp"
#include "util/uniform_uv.hpp"
#include "util/boundingbox.hpp"
#include "util/mesh.hpp"
#include "vertex.hpp"
#include "resources/resource.hpp"

namespace clover {
namespace visual {

class TriMesh;
	
} // visual
namespace resources {

class MeshSubCache;

template <>
struct ResourceTraits<visual::TriMesh> {
	DECLARE_RESOURCE_TRAITS(visual::TriMesh, String)
	
	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"))
	
	typedef MeshSubCache SubCacheType;
	

	static util::Str8 typeName(){ return "Mesh"; }
	static util::Str8 identifierKey(){ return "name"; }
	
	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static bool expired(const visual::TriMesh&){ return false; }
};

} // resources
namespace visual {

/** @class BaseMesh A virtual class for drawable mesh
*/
class BaseMesh {
public:
	BaseMesh(const uint32& ch): contentHash(&ch){}
	
	BaseMesh(const BaseMesh&)= delete;
	BaseMesh(BaseMesh&&)= delete;
	BaseMesh& operator=(const BaseMesh&)= delete;
	BaseMesh& operator=(BaseMesh&&)= delete;
	
	virtual ~BaseMesh(){}

	virtual void draw() const = 0;
	virtual util::BoundingBox<util::Vec2f> getBoundingBox() const = 0;
	
	const uint32& getContentHash() const { return *contentHash; }
	
private:
	/// Pointer to contentHash of GenericMesh
	const uint32* contentHash;
};

/** @class GpuMesh A mesh which can be uploaded to the gpu and drawn
*/
template <typename V, typename I = IndexTypeNone>
class GpuMesh : public BaseMesh, public util::GenericMesh<V, I> {
public:

	typedef util::GenericMesh<V, I> GenMeshBaseClass;
	typedef VertexArrayObject<V, I> VaoType;
	typedef I IndexType;
	using This= GpuMesh;

	GpuMesh();
	virtual ~GpuMesh();

	/// @brief Doesn't copy vertex array object or submeshiness, only data
	GpuMesh(const GenMeshBaseClass& b);
	GpuMesh(const GpuMesh&);
	GpuMesh(GpuMesh&&);

	GpuMesh& operator=(GpuMesh&&);
	GpuMesh& operator=(const GpuMesh&);

	void addSubMesh(GpuMesh& m);
	bool isSubMesh() const { return superMesh != nullptr; }

	void setUsage(hardware::GlState::VaoUsage u);

	/** @brief Called internally before flush of the submesh
		@param external_vao VAO of the parent mesh
		@param vertex_offset First vertex element dedicated to this mesh in the external_vao
		@param index_offset First index element dedicated to this mesh in the external_vao
	*/
	void onSubMeshFlush(VaoType* external_vao, uint32 vertex_offset, uint32 index_offset);

	/// @brief Clears only data, not VAO or submeshiness (there could be reset() for that)
	virtual void clear();
	
	/// Uploads data to the GPU
	virtual void flush() const;

	virtual void draw() const;

	virtual util::BoundingBox<util::Vec2f> getBoundingBox() const;
	
	/// @todo Remove, reveals mutable state
	const VaoType* getVao() const { return vao; }
	
	
protected:
	using GenMeshBaseClass::vertices;
	using GenMeshBaseClass::indices;
	using GenMeshBaseClass::isDirty;
	using GenMeshBaseClass::setDirty;

	/// Calls createVaoImpl with parameter hardware::GlState::Primitive::Triangle
	virtual void createVao() const;
	void createVaoImpl(hardware::GlState::Primitive p) const;
	virtual void fixDirtyness() const override;
	
private:
	void destroyVao() const;

	/// Internal vertex array object (or external, if subMesh is true)
	mutable VaoType* vao;

	/// Is this mesh a submesh (uses someone else's VAO)
	This* superMesh;
	mutable uint32 subVertexOffset, subIndexOffset;

	struct SubMeshInfo {
		GpuMesh* mesh;
		// Comparing these to current values submesh determines if it
		// can just overwrite its vertices instead of flushing the whole supermesh
		mutable uint32 vertexCount;
		mutable uint32 indexCount;
	};
	
	const SubMeshInfo& getSubMeshInfo(const GpuMesh& m) const;

	util::DynArray<SubMeshInfo> subMeshInfos;
};

/// @class TriMesh A drawable triangle mesh
class TriMesh : public GpuMesh<Vertex, uint32>, public resources::Resource {
public:
	DECLARE_RESOURCE(TriMesh)
	
	typedef uint32 IndexType;
	typedef GpuMesh<Vertex, IndexType> BaseClass;

	TriMesh(uint32 reserved_vert_count=4, uint32 reserved_ind_count= 6);
	TriMesh(const BaseClass& b);
	TriMesh(const BaseClass::GenMeshBaseClass& b);

	TriMesh(const TriMesh&);
	TriMesh(TriMesh&&);

	virtual ~TriMesh();

	/// @brief Doesn't copy vertex array object or submeshiness, only data
	TriMesh& operator=(const TriMesh& m);
	TriMesh& operator=(TriMesh&& m)= default;

	void calculateTangents();

	void add(util::Vec2f pos, util::Vec2d uv);
	void add(util::Vec2f pos, util::Color color=util::Color{1,1,1,1});
	void add(int32 vertcount, Vertex* data);

	void addTriangle(Vertex p[3]);
	void setIndices(const util::DynArray<IndexType>& ind);

	void add(const TriMesh& mesh);
	void addTriangles(const util::DynArray<util::Vec2d>& v);
	void addTriangles(const util::DynArray<util::Vec2f>& v);
	void addTriangle(util::Vec2f p1, util::Vec2f p2, util::Vec2f p3);
	void addTriangle(	const util::Vec2f& p1, const util::Vec2f& p2, const util::Vec2f& p3, 
						const util::Color& c1, const util::Color& c2, const util::Color& c3);


	void overwrite(int32 index, TriMesh& mesh);

	/// @param size "radius"
	void addRectByCenter(const util::Vec2f& pos, const util::Vec2f& size, const util::Vec2f& uvpos=util::Vec2f{0.5, 0.5}, const util::Vec2f& uvsize=util::Vec2f{0.5, 0.5});
	void addRectByCorners(const util::Vec2f& lower_left, const util::Vec2f& upper_right, const util::Vec2f& uv_lower_left=util::Vec2f{0,0}, const util::Vec2f& uv_upper_right=util::Vec2f{1,1});

	bool hasUniformUv() const { return uniformUvSet; }
	util::UniformUv getUniformUv() const { return uv; }
	void applyUniformUv(const util::UniformUv& uv);
	
	void applyUv(const TriMesh& other);

	/// @todo Move to GenericMesh
	void translate(util::Vec2f t);
	void rotate(real32 rot);
	void scale(util::Vec2f s);

	void setColor(const util::Color& c, int32 index=-1);

	/// @brief Clears mesh and submeshes
	virtual void clear();

	virtual void resourceUpdate(bool load, bool force=true);
	virtual void createErrorResource();
	
private:
	bool uniformUvSet;
	
	/// Could probably be deleted because everything is composed of single vertices now
	util::UniformUv uv;

	void initAttributeCallbacks();
	void calculateTangentTriangle(Vertex* tri[3]);
	
	/// For attributes
	static util::DynArray<Vertex> defaultVertices();
	static util::DynArray<IndexType> defaultIndices();
	
	RESOURCE_ATTRIBUTE(String, nameAttribute)
	RESOURCE_ATTRIBUTE(VertexArray, verticesAttribute)
	RESOURCE_ATTRIBUTE(TriIndices, indicesAttribute)
};

template <typename V>
class PointMesh : public GpuMesh<V> {
public:
	typedef GpuMesh<V> BaseClass;

	PointMesh();
	virtual ~PointMesh();

	PointMesh(const PointMesh&)= default;
	PointMesh(PointMesh&&)= default;

	PointMesh& operator=(PointMesh&&)= default;
	PointMesh& operator=(const PointMesh&)= default;

	void setDrawRange(uint32 offset, uint32 count);

	virtual void draw() const;

protected:
	virtual void createVao() const override;

private:
	uint32 drawOffset, drawCount;
};

#include "visual/mesh.tpp"

} // visual
} // clover

#endif // CLOVER_VISUAL_MESH_IMPL_HPP
