#ifndef CLOVER_VISUAL_ENTITY_DEF_MODEL_HPP
#define CLOVER_VISUAL_ENTITY_DEF_MODEL_HPP

#include "build.hpp"
#include "resources/resource.hpp"
#include "util/coord.hpp"
#include "util/math.hpp"
#include "util/pooled_crtp.hpp"
#include "visual/material.hpp"
#include "visual/model.hpp"
#include "visual/entity_def.hpp"

#include <GL/glew.h>

namespace clover {
namespace visual {

class ModelEntityDef;

} // visual
namespace resources {

template <>
struct ResourceTraits<visual::ModelEntityDef> {
	DECLARE_RESOURCE_TRAITS(visual::ModelEntityDef, String)
	
	typedef ResourceTraits<visual::EntityDef> BaseTraits;
	
	static util::DynArray<AttributeDef> getAttributeDefs(){
		return	std::move(
					BaseTraits::getAttributeDefs().pushBacked(util::DynArray<AttributeDef> {
						AttributeDef::Resource("model"),
						AttributeDef::Real("envLight"),
						AttributeDef::Boolean("usesOnlyEnvLight"),
						AttributeDef::Boolean("shadowCasting"),
						AttributeDef::Boolean("billboard")
					})
				);
	}
	
	/// @todo custom cache
	typedef VisualDerivedEntityDefSubCache<visual::ModelEntityDef> SubCacheType;
	
	static util::Str8 typeName(){ return "ModelEntityDef"; }
	static util::Str8 identifierKey(){ return "name"; }
	
	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static bool expired(const visual::ModelEntityDef& def){ return false; }
};

} // resources
namespace visual {

class EntityMgr;
class TextModel;

/// @class ModelEntity Model instance
/// @todo Move stuff to Material
class ModelEntityDef	: public EntityDef
						/*, public util::PooledCrtp<ModelEntityDef>*/ {
public:
	DECLARE_RESOURCE(ModelEntityDef)
	
	enum ShadingType {
		Shading_Generic,
		Shading_Particle, 
		Shading_Fluid,
		ShadingType_Last
	};

	ModelEntityDef();
	ModelEntityDef(const ModelEntityDef&)= default;
	ModelEntityDef(ModelEntityDef&&)= default;

	ModelEntityDef& operator=(const ModelEntityDef& re)= default;
	ModelEntityDef& operator=(ModelEntityDef&&)= default;

	virtual ~ModelEntityDef();

	void setModel(const Model& m);
	/// @brief Same as setModel, but sets scale to pixel-units and enables snapToPixels
	//void setPixelModel(const Model& m);
	void setModel(const util::Str8& str);

	const Model* getModel() const { return model; }

	DrawPriority getDrawPriority() const { return drawPriority; }

	void setShadingType(ShadingType shdtype);
	ShadingType getShadingType() const;

	void setEnvLight(real32 intensity);
	bool usesCustomEnvLight() const;
	real32 getCustomEnvLight() const;
	
	void useOnlyEnvLight(bool e=true);
	bool usesOnlyEnvLight() const;

	void setColorMul(const util::Color& c){ colorMul= c; }

	util::Color getColorMul() const;

	Material::BlendFunc getBlendFunc() const { return model ? model->getBlendFunc() : Material::defaultBlendFunc; }

	void setShadowCasting(bool b=true);
	bool isShadowCasting() const;

	void setLightAlphaAdd(real32 f);
	real32 getLightAlphaAdd() const;

	void setFilled(bool fill);
	bool isFilled() const;

	void setSnappingToPixels(bool b=true){ snapToPixels= b; }
	bool isSnappingToPixels() const { return snapToPixels; }
	
	// Moves vertices vertically
	void setSwaying(bool b=true){ sway= b; }
	bool isSwaying() const { return sway; }

	void setSwayPhaseMul(real32 mul){ swayPhaseMul= mul; }
	real32 getSwayPhaseMul() const { return swayPhaseMul; }

	void setSwayScale(real32 scale){ swayScale= scale; }
	real32 getSwayScale() const { return swayScale; }

	bool isBillboard() const { return billboardAttribute.get(); }

	void draw() const;

	virtual EntityLogic* createLogic() const;
	
	virtual void resourceUpdate(bool load, bool force=true) override;
	virtual void createErrorResource() override;
	
	uint32 getContentHash() const;
	uint32 getBatchCompatibilityHash() const;
	
private:
	friend class EntityMgr;
	friend class ShaderTech;

	ShadingType shadingType;
	bool filled;
	// if position is rounded to pixel-grid
	bool snapToPixels;
	util::Color colorMul;
	real32 lightAlphaAdd;
	const Model* model;
	// Determines drawing order if z difference <= util::epsilon
	DrawPriority drawPriority;
	bool sway;
	real32 swayPhaseMul;
	real32 swayScale;

	RESOURCE_ATTRIBUTE(Resource, modelAttribute);
	// Negative means that use global
	RESOURCE_ATTRIBUTE(Real, envLightAttribute);
	// If true, dynamic lighting is ignored
	RESOURCE_ATTRIBUTE(Boolean, onlyEnvAttribute);
	RESOURCE_ATTRIBUTE(Boolean, castsShadowAttribute);
	RESOURCE_ATTRIBUTE(Boolean, billboardAttribute);	
};

} // visual
} // clover

#endif // CLOVER_VISUAL_ENTITY_DEF_MODEL_HPP
