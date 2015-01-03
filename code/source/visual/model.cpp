#include "global/env.hpp"
#include "model.hpp"
#include "resources/cache.hpp"
#include "visual/material.hpp"
#include "visual/mesh.hpp"

namespace clover {
namespace visual {

Model::Model():
		material(0),
		mesh(0),
		INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", ""),
		INIT_RESOURCE_ATTRIBUTE(materialAttribute, "material", ""),
		INIT_RESOURCE_ATTRIBUTE(meshAttribute, "mesh", ""){
	
			
	materialAttribute.setOnChangeCallback([=] (){
		if (materialAttribute.get().empty())
			material= 0;
		else
			material= &global::g_env->resCache->getResource<visual::Material>(materialAttribute.get());
	});
	
	meshAttribute.setOnChangeCallback([=] (){
		if (meshAttribute.get().empty())
			mesh= 0;
		else
			mesh= &global::g_env->resCache->getResource<visual::TriMesh>(meshAttribute.get());
	});
}

void Model::setMaterial(const util::Str8& name){
	material= &global::g_env->resCache->getResource<visual::Material>(name);
}

void Model::setMesh(const BaseMesh& m){
	mesh= &m;
}

void Model::setMesh(const util::Str8& name){
	mesh= &global::g_env->resCache->getResource<visual::TriMesh>(name);
}

util::BoundingBox<util::Vec3f> Model::getBoundingBox() const {
	if(!mesh)
		return util::BoundingBox<util::Vec3f>();
	return mesh->getBoundingBox();
}

void Model::resourceUpdate(bool load, bool force){
	materialChangeListener.clear();
	meshChangeListener.clear();
	
	if (load || getResourceState() == State::Uninit){
		material= &global::g_env->resCache->getResource<Material>(materialAttribute.get());
		mesh= &global::g_env->resCache->getResource<TriMesh>(meshAttribute.get());
		
		materialChangeListener.listen(*material, [=] (){ util::OnChangeCb::trigger(); });
		
		const visual::TriMesh* trimesh= dynamic_cast<const visual::TriMesh*>(mesh);
		if (trimesh)
			meshChangeListener.listen(*trimesh, [=] (){ util::OnChangeCb::trigger(); });
		
		setResourceState(State::Loaded);
	}
	else {
		setResourceState(State::Unloaded);
	}
}

void Model::createErrorResource(){
	setResourceState(State::Error);

	material= &global::g_env->resCache->getErrorResource<Material>();
	mesh= &global::g_env->resCache->getErrorResource<TriMesh>();
}

uint32 Model::getContentHash() const {
	if (!mesh || !material)
		return 0;
	else
		return getBatchCompatibilityHash() + mesh->getContentHash();
}

uint32 Model::getBatchCompatibilityHash() const {
	return material ? material->getContentHash() : 0;
}

} // visual
} // clover
