#ifndef CLOVER_VISUAL_ENTITYLOGIC_MODEL_HPP
#define CLOVER_VISUAL_ENTITYLOGIC_MODEL_HPP

#include "build.hpp"
#include "entitylogic.hpp"
#include "entity_def_model.hpp"

namespace clover {
namespace visual {

/// Renderable instance of a model
class ModelEntityLogic : public EntityLogic {
public:
	using Base= EntityLogic;
	using BoundingBox= util::BoundingBox<util::Vec3d>;

	ModelEntityLogic(const ModelEntityDef& def);
	ModelEntityLogic(const ModelEntityLogic&)= default;
	ModelEntityLogic(ModelEntityLogic&&)= default;
	virtual ~ModelEntityLogic();

	ModelEntityLogic& operator=(const ModelEntityLogic&)= default;
	ModelEntityLogic& operator=(ModelEntityLogic&&)= default;

	const ModelEntityDef& getDef() const { return *static_cast<const ModelEntityDef*>(&EntityLogic::getDef()); }

	virtual void setDrawPriority(DrawPriority p) override;
	DrawPriority getDrawPriority() const;

	virtual void apply(const EntityLogic& other) override;

	/// @return Bounding box (not translated to position)
	///			Guaranteed to be set
	BoundingBox getBoundingBox() const;

	void setColorMul(const util::Color& c){ colorMul= c; }
	const util::Color& getColorMul() const { return colorMul; }

	uint32 getContentHash() const;

	/// Same for entities which could belong to the same rendering batch
	uint32 getBatchCompatibilityHash() const;

private:
	bool useCustomDrawPriority;
	visual::ModelEntityDef::DrawPriority customDrawPriority;

	util::Color colorMul;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_ENTITYLOGIC_MODEL_HPP
