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
			material= &resources::gCache->getResource<visual::Material>(materialAttribute.get());
	});
	
	meshAttribute.setOnChangeCallback([=] (){
		if (meshAttribute.get().empty())
			mesh= 0;
		else
			mesh= &resources::gCache->getResource<visual::TriMesh>(meshAttribute.get());
	});
}

void Model::setMaterial(const util::Str8& name){
	material= &resources::gCache->getResource<visual::Material>(name);
}

void Model::setMesh(const BaseMesh& m){
	mesh= &m;
}

void Model::setMesh(const util::Str8& name){
	mesh= &resources::gCache->getResource<visual::TriMesh>(name);
}

util::BoundingBox<util::Vec2f> Model::getBoundingBox() const {
	if(!mesh) return util::BoundingBox<util::Vec2f>();
	return mesh->getBoundingBox();
}


void Model::resourceUpdate(bool load, bool force){
	materialChangeListener.clear();
	meshChangeListener.clear();
	
	if (load || getResourceState() == State::Uninit){
		material= &resources::gCache->getResource<Material>(materialAttribute.get());
		mesh= &resources::gCache->getResource<TriMesh>(meshAttribute.get());
		
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

	material= &resources::gCache->getErrorResource<Material>();
	mesh= &resources::gCache->getErrorResource<TriMesh>();
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