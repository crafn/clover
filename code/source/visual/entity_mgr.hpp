#ifndef CLOVER_VISUAL_ENTITY_MGR
#define CLOVER_VISUAL_ENTITY_MGR

#include "build.hpp"
#include "entity_def_model.hpp"
#include "global/eventreceiver.hpp"
#include "model.hpp"
#include "renderinganalyzer.hpp"
#include "shadertech_generic.hpp"
#include "shadertech_fluid.hpp"
#include "shadertech_shadowcaster.hpp"
#include "shadertech_shadowmap.hpp"
#include "shadertech_particle.hpp"
#include "util/finitegridcontainer.hpp"
#include "util/map.hpp"
#include "util/hashset.hpp"

namespace clover {
namespace visual {

class LightEntityLogic;
class ModelEntityLogic;
class ShaderMgr;

/// @class EntityMgr Manages visual::Entities and renders them (rendering should be separated)
/// @todo Split renderer to separate class
class EntityMgr : public global::EventReceiver {
public:
	EntityMgr(ShaderMgr& shader_mgr);
	virtual ~EntityMgr();

	/// Draws a frame with current camera
	void draw();

	int32 getModelEntityCount();

	void setEnvLight(util::Color f, util::Vec2f dir);
	void onEvent(global::Event& e);
	
	void onEntityActivationChange(Entity& e, bool active);
	void onEntitySpatialChange(Entity& e, const Entity::Transform& prev_t);
	void onEntityCoordSpaceChange(
			Entity& e,
			util::Coord::Type next_pos_space,
			util::Coord::Type next_scale_space);

private:
	struct RenderEntity {
		const visual::ModelEntityDef* def;
		const visual::ModelEntityLogic* logic;
		

		util::Vec2d translation;
		util::Quatd rotation;
		util::Vec2d scale;
		
		/// If not visible, shouldn't be drawn
		bool visible;
		
		/// Call after modifying
		void finalize();

		/// Drawing order information
		/// util::Set in finalize()
		EntityLogic::Layer layer;
		real64 depth;
		EntityDef::DrawPriority priority;
	};
	
	struct RenderFrameConfig {
		util::DynArray<RenderEntity> entities;
				
		util::DynArray<LightEntityLogic*> lights;
		Camera* camera= nullptr;
		real64 occlusionRadiusAddition= 0.0;

		/// Call after modifying
		void finalize();
		
		/// Filled in finalize
		util::Map<const ModelEntityLogic*, const RenderEntity*> logicToRenderEntity;
	};
	
	struct RenderBatch {
		RenderBatch(): logic(def){}
		RenderBatch(const RenderBatch&)= delete;
		RenderBatch(RenderBatch&&);

		RenderEntity asRenderEntity() const;

		/// Merged mesh
		TriMesh mesh;
		Model model;
		ModelEntityDef def;
		ModelEntityLogic logic;
		const ModelEntityLogic* lastEntityLogic= nullptr;
	};
	
	class RenderEntityCache {
	public:
		using Grid= util::FiniteGridContainer<const ModelEntityLogic*, 3>;

		RenderEntityCache(real64 chunk_size): chunkSize(chunk_size){}
		void add(const ModelEntityLogic& logic);
		void remove(const ModelEntityLogic& logic);
		void onSpatialChange(
				const ModelEntityLogic& logic,
				const Entity::Transform& prev_t);
		void onCoordSpaceChange(
				const ModelEntityLogic& logic,
				util::Coord::Type next_space,
				util::Coord::Type next_scale_space);
		
		util::DynArray<RenderEntity> query(	Camera& cam,
										util::Vec2d occlusion_rad_addition);
		
		SizeType getModelEntityCount() const { return modelEntityCount; }
		
	private:
		Grid::Vec gridPos(util::Vec3d p) const;
		void addToGrid(const ModelEntityLogic& logic);
	
		real64 chunkSize;
		util::DynArray<const ModelEntityLogic*> viewSpace;
		/// Contains entities in a 3d grid
		Grid spatialGrid;
		SizeType modelEntityCount= 0;
	};
	
	struct ReSortByDepth {
		bool operator()(const RenderEntity& a, const RenderEntity& b);
	};

	/// @todo Purify these functions	
	RenderFrameConfig createSimpleRenderFrameConfig();
	void calcLighting(const RenderFrameConfig& cfg);
	static util::DynArray<RenderEntity> sortedByDepth(util::DynArray<RenderEntity> entities);
	void processAnalysis(const RenderingAnalyzer::Analysis& a, const RenderFrameConfig& cfg);
	void optimizeRenderFrameConfig(RenderFrameConfig& cfg);
	
	static RenderEntity minimalRenderEntity(const ModelEntityLogic& logic);

	util::DynArray<LightEntityLogic*> lights;

	GenericST		genericST;
	ShadowCasterST	shadowCasterST;
	ShadowMapST		shadowMapST;
	ParticleST		particleST;
	FluidST			fluidST;

	util::Color envLight;
	util::Vec2f envLightDir;
	
	RenderingAnalyzer analyzer;
	
	/// <Batch contentHash, batch>
	util::Map<uint32, RenderBatch> batchMap;
	/// <Entity contentHash, batch*>
	util::Map<uint32, RenderBatch*> entityToBatch;
	
	RenderEntityCache reCache;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_ENTITY_MGR
