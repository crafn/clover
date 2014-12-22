#ifndef CLOVER_UTIL_MESH_HPP
#define CLOVER_UTIL_MESH_HPP

#include "build.hpp"
#include "dyn_array.hpp"
#include "boundingbox.hpp"

namespace clover {
namespace util {

template <typename T>
struct MeshVertexTraits {
	typedef T BoundingBoxType;
	static T extractPosition(const T& v){ return v; }

	/// @brief Conversion, other to type T
	template <typename P>
	static T converted(const P& input){ return MeshVertexTraits<P>::extractPosition(input).template converted<T>(); }

};

/** @class GenericMesh A base class for all meshes
*/
template <typename V, typename I>
class GenericMesh {
public:
	typedef V VType; // Vertex type
	typedef I IType; // Index type
	typedef typename MeshVertexTraits<V>::BoundingBoxType BType; // util::BoundingBox type


	GenericMesh();
	GenericMesh(const GenericMesh&)= default;
	GenericMesh(GenericMesh&&)= default;

	GenericMesh& operator=(const GenericMesh&)= default;
	GenericMesh& operator=(GenericMesh&&)= default;

	/// Conversion to be used like converted<A,B>()
	template <typename VV, typename II>
	GenericMesh<VV, II> converted() const;

	/// Conversion to be used like converted<Mesh<A,B>>()
	template <class M>
	M converted() const;

	/// Adds a vertex to the mesh
	void addVertex(const VType& v);

	/// Adds a triangle to the mesh (also indices)
	void addTriangle(const VType& a, const VType& b, const VType& c);

	const VType& getVertex(const IType& i) const;
	void setVertex(const IType& i, const VType& v);
	void setVertices(util::DynArray<V> v);
	uint32 getVertexCount() const { return vertices.size(); }

	/// Adds a triangle between existing vertices
	void addTriangle(const IType& a, const IType& b, const IType& c);

	/// Adds a index to the mesh (remember to have indices.size() % 3 == 0)
	void addIndex(const IType& i);

	const IType& getIndex(uint32 i) const;
	void setIndex(uint32 i, const IType& ind);
	void setIndices(util::DynArray<I> i);
	uint32 getIndexCount() const { return indices.size(); }

	/// @brief If needed, calculates and returns a bounding box
	util::BoundingBox<BType> getBoundingBox() const;

	/// @brief Sets bounding box. (Not permanently)
	void setBoundingBox(const util::BoundingBox<BType>& bb);

	/// @brief Removes vertices and indices, and resets bounding box
	virtual void clear();

	const uint32& getContentHash() const;

protected:
	bool isDirty() const { return dirty; }
	void setDirty() const { dirty= true; }

	virtual void fixDirtyness() const;

	util::DynArray<VType> vertices;
	util::DynArray<IType> indices;

	// Mutables since getBoundingBox might recalculate bounding box
	mutable util::BoundingBox<BType> boundingBox;

private:
	mutable bool dirty= false;
	mutable uint32 contentHash= 0;
};

#include "mesh.tpp"

} // util
} // clover

#endif // CLOVER_UTIL_MESH_HPP
