#ifndef CLOVER_VISUAL_MODEL_HPP
#define CLOVER_VISUAL_MODEL_HPP

#include "build.hpp"
#include "material.hpp"
#include "resources/resource.hpp"
#include "util/cb_listener.hpp"
#include "util/boundingbox.hpp"
#include "util/pooled_crtp.hpp"
#include "util/string.hpp"

namespace clover {
namespace visual {

class Model;

} // visual
namespace resources {

template <>
struct ResourceTraits<visual::Model> {
	DECLARE_RESOURCE_TRAITS(visual::Model, String)
	
	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"),
							AttributeDef::Resource("material", "Material"),
							AttributeDef::Resource("mesh", "Mesh"))
	
	typedef SubCache<visual::Model> SubCacheType;

	static util::Str8 typeName(){ return "Model"; }
	static util::Str8 identifierKey(){ return "name"; }

	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static bool expired(const visual::Model&){ return false; }
};

} // resources
namespace visual {

class BaseMesh;
class Material;

/// Drawable model. Contains material and mesh
class Model :	public resources::Resource
			,	public util::PooledCrtp<Model> {
public:
	DECLARE_RESOURCE(Model)
	
	Model();
	Model(const Model&)= default;
	Model(Model&&)= default;
	virtual ~Model() {}

	Model& operator=(Model&&)= default;
	Model& operator=(const Model&)= default;

	void setMaterial(const Material& m){ material= &m; }
	void setMaterial(const util::Str8& name);

	/// Sets mesh of the model. Remember to flush the mesh
	void setMesh(const BaseMesh& m);
	void setMesh(const util::Str8& name);

	const Material* getMaterial() const { return material; }
	const BaseMesh* getMesh() const { return mesh; }

	Material::BlendFunc getBlendFunc() const { return material ? material->getBlendFunc() : Material::defaultBlendFunc; }

	/// Returns bounding box of the mesh
	util::BoundingBox<util::Vec3f> getBoundingBox() const;

	virtual void resourceUpdate(bool load, bool force=true) override;
	virtual void createErrorResource() override;
	
	uint32 getContentHash() const;
	uint32 getBatchCompatibilityHash() const;

protected:
	const Material* material;
	const BaseMesh* mesh;
	
	RESOURCE_ATTRIBUTE(String, nameAttribute)
	RESOURCE_ATTRIBUTE(Resource, materialAttribute)
	RESOURCE_ATTRIBUTE(Resource, meshAttribute)
	
	util::CbListener<util::OnChangeCb> materialChangeListener;
	util::CbListener<util::OnChangeCb> meshChangeListener;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_MODEL_HPP
