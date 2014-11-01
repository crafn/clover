#ifndef CLOVER_VISUAL_SUBCACHE_MESH_HPP
#define CLOVER_VISUAL_SUBCACHE_MESH_HPP

#include "build.hpp"
#include "resources/subcache.hpp"
#include "visual/mesh_impl.hpp"

namespace clover {
namespace resources {

class MeshSubCache : public SubCache<visual::TriMesh> {
public:
	typedef visual::TriMesh MeshType;
	
	virtual ~MeshSubCache();
	virtual void preLoad();
	
protected:
	virtual MeshType& createNewResource(const util::Str8& key);

private:
	MeshType staticMesh;
};

} // resources
} // clover

#endif // CLOVER_VISUAL_SUBCACHE_MESH_HPP