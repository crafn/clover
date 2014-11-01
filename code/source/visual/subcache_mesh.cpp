#include "subcache_mesh.hpp"

namespace clover {
namespace resources {

MeshSubCache::~MeshSubCache(){
}

void MeshSubCache::preLoad(){
	SubCache<visual::TriMesh>::preLoad();
	staticMesh.flush();
}

MeshSubCache::MeshType& MeshSubCache::createNewResource(const util::Str8& key){
	MeshType& mesh= SubCache<MeshType>::createNewResource(key);
	staticMesh.addSubMesh(mesh);
	return mesh;
}

} // resource
} // clover