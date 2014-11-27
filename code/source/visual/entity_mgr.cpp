#include "entity_mgr.hpp"
#include "camera.hpp"
#include "camera_mgr.hpp"
#include "debug/debugdraw.hpp"
#include "entitylogic_light.hpp"
#include "entity_def_light.hpp"
#include "entity_def_model.hpp"
#include "entitylogic_compound.hpp"
#include "entitylogic_light.hpp"
#include "entitylogic_model.hpp"
#include "global/cfg_mgr.hpp"
#include "hardware/device.hpp"
#include "hardware/glstate.hpp"
#include "model_text.hpp"
#include "util/profiling.hpp"
#include "visual_mgr.hpp"
// Debug
#include "hardware/keyboard.hpp"

#include <set>
#include <chrono>

namespace clover {
namespace visual {

util::SrtTransform3d billboarded(util::SrtTransform3d t){
	util::Vec3d normal= util::Vec3d{0, 0, 1}*t.rotation;
	util::Vec3d wanted_normal= util::Vec3d{0, 0, normal.z > 0 ? 1.0 : -1.0};
	t.rotation= t.rotation*util::Quatd::byRotationFromTo(normal, wanted_normal);
	return t;
}

EntityMgr::EntityMgr(ShaderMgr& shader_mgr)
	: reCache(global::gCfgMgr->get("visual::gridCellSize", 16.0))
{
	envLight= 0.0;

	genericST.setShaderMgr(shader_mgr);
	shadowCasterST.setShaderMgr(shader_mgr);
	shadowMapST.setShaderMgr(shader_mgr);
	particleST.setShaderMgr(shader_mgr);
	fluidST.setShaderMgr(shader_mgr);
}

EntityMgr::~EntityMgr(){
	ModelEntityLogic::setPoolMem(nullptr);
	ModelEntityDef::setPoolMem(nullptr);
}

void EntityMgr::draw(){
	RenderFrameConfig cfg;
	{ PROFILE();
		cfg= createSimpleRenderFrameConfig();
		// Analyze frame before rendering
		analyzer.onFrameStart();
		for (uint32 i= 0; i < cfg.entities.size(); i++){
			ensure(cfg.entities[i].logic);
			analyzer.onDraw(*cfg.entities[i].logic);
		}
		processAnalysis(analyzer.analyze(), cfg);
		optimizeRenderFrameConfig(cfg);

		calcLighting(cfg);
	}

	{ PROFILE();

		// Actual rendering of visual entities
		hardware::gGlState->bindFbo(0);
		hardware::gGlState->setViewport({0,0}, hardware::gDevice->getViewportSize());

		genericST.setCamera(*cfg.camera);
		genericST.setLights(cfg.lights);
		particleST.setCamera(*cfg.camera);

		auto set_lights= [&] (const visual::ModelEntityDef& def){
			if (def.usesOnlyEnvLight())
				genericST.setLightCount(0);
			else
				genericST.setLightCount(cfg.lights.size());
			
		};

		SizeType draw_count= 0;
		for (SizeType i= 0; i < cfg.entities.size(); i++){
			const visual::ModelEntityDef& re= *cfg.entities[i].def;
			const visual::ModelEntityLogic& logic= *cfg.entities[i].logic;
			
			++draw_count;

			util::Color env_light= envLight;
			util::Vec2f env_light_dir= envLightDir;

			if (re.usesCustomEnvLight())
				env_light= re.getCustomEnvLight();

			if (re.getShadingType() == visual::ModelEntityDef::Shading_Generic){
				PROFILE();
				set_lights(re);
				genericST.setEnvLight(env_light, env_light_dir);
				genericST.setEntity(re, logic);
				genericST.setTransform(cfg.entities[i].transform);
				genericST.use();
				re.draw();
			}
			else if (re.getShadingType() == visual::ModelEntityDef::Shading_Particle){
				PROFILE();
				particleST.setEntity(re);
				genericST.setTransform(cfg.entities[i].transform);
				genericST.use();
				particleST.use();
				re.draw();
			}
			else if (re.getShadingType() == visual::ModelEntityDef::Shading_Fluid){
				fluidST.render(*cfg.camera);
			}
		}

		//print(debug::Ch::Visual, debug::Vb::Trivial, "draw_count: %zu", draw_count);
	}
}

int32 EntityMgr::getModelEntityCount(){
	return reCache.getModelEntityCount();
}

void EntityMgr::setEnvLight(util::Color f, util::Vec2f dir){
	envLight= f;
	envLightDir= dir;
}

void EntityMgr::onEvent(global::Event& e){
	switch(e.getType()){
		default: ensure(0);
	}
}

void EntityMgr::onEntityActivationChange(Entity& e, bool active){
	PROFILE_("render");

	ensure(e.getDef() && e.getLogic());
	switch (e.getDef()->getType()){
		case EntityDef::Type::None: break;

		case EntityDef::Type::Model: {
			const auto& logic= *static_cast<ModelEntityLogic*>(e.getLogic());
			if (active)
				reCache.add(logic);
			else
				reCache.remove(logic);
		}
		break;
		
		case EntityDef::Type::Compound: {
			auto& logic= *static_cast<CompoundEntityLogic*>(e.getLogic());
			logic.onSetActive(active);
		}
		break;
		
		case EntityDef::Type::Light: {
			auto& logic= *static_cast<LightEntityLogic*>(e.getLogic());
			if (active){
				lights.pushBack(&logic);
			}
			else {	
				auto it= lights.find(&logic);
				ensure(it != lights.end());
				lights.erase(it);
			}
		}
		break;
		
		case EntityDef::Type::ParticleSource: {
			print(debug::Ch::Visual, debug::Vb::Critical, "@todo particle sources");
		}
		break;
		
		default: ensure_msg(0, "illegal entityDef type: %i", (int32)e.getDef()->getType());
	}
}

void EntityMgr::onEntitySpatialChange(Entity& e, const Entity::Transform& prev_t){
	PROFILE_("render");
	if (e.getDef()->getType() == EntityDef::Type::Model){
		reCache.onSpatialChange(
				*static_cast<ModelEntityLogic*>(e.getLogic()), prev_t); 
	}
}

void EntityMgr::onEntityCoordSpaceChange(
		Entity& e,
		util::Coord::Type next_pos_space,
		util::Coord::Type next_scale_space){
	PROFILE_("render");
	if (e.getDef()->getType() == EntityDef::Type::Model){
		reCache.onCoordSpaceChange(
				*static_cast<ModelEntityLogic*>(e.getLogic()),
				next_pos_space,
				next_scale_space);
	}
}

//
// RenderEntity
//

void EntityMgr::RenderEntity::finalize(){
	layer= logic->getLayer();
	depth= (logic->getDef().getOffset()*logic->getTransform()).translation.z;
	priority= logic->getDrawPriority();
}

//
// RenderFrameConfig
//

void EntityMgr::RenderFrameConfig::finalize(){
	logicToRenderEntity.clear();
	for (const RenderEntity& e : entities){
		logicToRenderEntity[e.logic]= &e;
	}
}

//
// RenderBatch
//

EntityMgr::RenderBatch::RenderBatch(RenderBatch&& other)
		: mesh(std::move(other.mesh))
		, model(std::move(other.model))
		, def(std::move(other.def))
		, logic(std::move(other.logic))
		, lastEntityLogic(other.lastEntityLogic){
	model.setMesh(mesh);
	def.setModel(model);
	logic.setDef(def);
}

EntityMgr::RenderEntity EntityMgr::RenderBatch::asRenderEntity() const {
	RenderEntity re= {};
	re.def= &def;
	re.logic= &logic;
	re.visible= true;
	re.finalize();
	return re;
}

//
// RenderFrameConfig
//

EntityMgr::RenderFrameConfig EntityMgr::createSimpleRenderFrameConfig(){
	PROFILE();
	RenderFrameConfig cfg;
	
	if (visual::gVisualMgr->getCameraMgr().getActiveCameras().size() == 0)
		throw global::Exception("REMgr::draw(): No camera specified");
	
	Camera* camera= gVisualMgr->getCameraMgr().getActiveCameras()[0];
	ensure(camera);
	camera->setPerspectiveMul(global::gCfgMgr->get("visual::perspectiveMul", 1.0f));
	
	cfg.camera= camera;

	for (uint32 i=0; i<lights.size(); i++){
		// A magic multiplier to prevent instant light disappearing outside screen
		/// @todo Calculate multiplier from intensity, e.g. light disappears when max_intensity_on_screen < 0.01
		real64 rad= lights[i]->getDef().getHalfValueRadius()*4.0;
		
		util::Coord coord(lights[i]->getLightTransform().translation.xy(), lights[i]->getCoordinateSpace());
		if (camera->isWorldBBInViewport(coord.converted(
				util::Coord::World).getValue(),
				util::Vec2d(rad))){
			cfg.lights.pushBack(lights[i]);
			
			// Drawing boundaries are made larger when there's lights around so that shadows work properly
			if (cfg.occlusionRadiusAddition < rad){
				cfg.occlusionRadiusAddition= rad;
			}
		}
	}
	cfg.occlusionRadiusAddition += global::gCfgMgr->get<real64>("visual::occlusionRadiusAddition", 0.0);
	
	cfg.entities=
		sortedByDepth(
			reCache.query(	*camera,
							util::Vec2d(cfg.occlusionRadiusAddition)
			)
		);
	cfg.finalize();

	return cfg;
}

//
// RenderEntityCache
//

bool inWorldSpace(const ModelEntityLogic& logic){
	return	logic.getCoordinateSpace() == util::Coord::World &&
			logic.getScaleCoordinateSpace() == util::Coord::World;
}

void EntityMgr::RenderEntityCache::add(const ModelEntityLogic& logic){
	if (inWorldSpace(logic)){
		addToGrid(logic);
	}
	else {
		viewSpace.pushBack(&logic);
	}

	++modelEntityCount;
}

void EntityMgr::RenderEntityCache::remove(const ModelEntityLogic& logic){
	if (inWorldSpace(logic))
		spatialGrid.remove(&logic, gridPos(logic.getTransform().translation));
	else
		viewSpace.remove(&logic);

	--modelEntityCount;
}

void EntityMgr::RenderEntityCache::onSpatialChange(
		const ModelEntityLogic& logic,
		const Entity::Transform& prev_t){
	if (!inWorldSpace(logic))
		return;

	ModelEntityLogic::BoundingBox bb= logic.getBoundingBox();

	Grid::Vec prev_grid_pos= gridPos(prev_t.translation);
	util::Vec3d cur_pos= logic.getTransform().translation;
	Grid::Vec cur_grid_origin= gridPos(cur_pos + bb.getMin().xyz());
	Grid::Vec cur_grid_end= gridPos(cur_pos + bb.getMax().xyz()) + Grid::Vec(1);
	Grid::Vec cur_grid_size= cur_grid_end - cur_grid_origin;

	// Relying on the fact that position of an entity is inside its bounding box
	ensure(	bb.getMin().componentsLessOrEqualThan(util::Vec3d(0)) &&
			bb.getMax().componentsGreaterOrEqualThan(util::Vec3d(0)));

	const Grid::Element& prev_grid_element= spatialGrid.get(&logic, prev_grid_pos);

	if (	prev_grid_element.origin != cur_grid_origin ||
			prev_grid_element.size != cur_grid_size){
		spatialGrid.remove(&logic, prev_grid_pos);
		spatialGrid.add(&logic, cur_grid_origin, cur_grid_size);
	}
}

void EntityMgr::RenderEntityCache::onCoordSpaceChange(
		const ModelEntityLogic& logic,
		util::Coord::Type next_pos_space,
		util::Coord::Type next_scale_space){
	PROFILE();

	if (	inWorldSpace(logic) &&
			next_pos_space == util::Coord::World && next_scale_space == util::Coord::World)
		return;
	if (	!inWorldSpace(logic) &&
			(next_pos_space != util::Coord::World || next_scale_space != util::Coord::World))
		return;

	remove(logic);
	
	if (next_pos_space == util::Coord::World && next_scale_space == util::Coord::World)
		addToGrid(logic);
	else
		viewSpace.pushBack(&logic);
}

util::DynArray<EntityMgr::RenderEntity> EntityMgr::RenderEntityCache::query(
		const Camera& camera,
		util::Vec2d occlusion_rad_addition){
	PROFILE();
		
	bool show_gui= global::gCfgMgr->get("visual::showGui", true);

	util::Vec2d campos= camera.getPosition();
	real32 camscale= camera.getScale();

	// Radius of viewport in world coordinates
	util::Vec2d rad= util::Vec2d(1.0)/camera.getScale();
	rad.x /= hardware::gDevice->getAspectVector().x;
	rad.y /= hardware::gDevice->getAspectVector().y;

	util::Vec2d aspectvector= hardware::gDevice->getAspectVector();
	util::Vec2d inverse_aspectvector= util::Vec2d{1.0/aspectvector.x, 1.0/aspectvector.y};
	real64 aspect_component_dif= util::abs(aspectvector.x-aspectvector.y);

	util::Vec2d viewport= util::Vec2d{(real32)hardware::gDevice->getViewportSize().x*0.5, (real32)hardware::gDevice->getViewportSize().y*0.5};

	bool viewport_oddx= false;
	bool viewport_oddy= false;
	if (hardware::gDevice->getViewportSize().x % 2 == 1) viewport_oddx= true;
	if (hardware::gDevice->getViewportSize().y % 2 == 1) viewport_oddy= true;

	// Multiply by this to get from world size to pixel size
	util::Vec2d pixelmul= {	1.0/viewport.x/aspectvector.x,
							1.0/viewport.y/aspectvector.y};

	pixelmul= pixelmul/camscale;

	auto visibility_update= [&] (EntityMgr::RenderEntity& re){
		PROFILE();
		ensure(re.logic);
		ensure(re.def);
		const ModelEntityLogic& logic= *re.logic;
		{ PROFILE();

			re.visible= false;
			
			if (!show_gui && logic.getLayer() == EntityLogic::Layer::Gui)
				return;
			
			if (!re.def->getModel())
				return;
		}
		
		util::SrtTransform3d offset, transform;

		{ PROFILE();
			offset= logic.getDef().getOffset();
			transform= offset*logic.getTransform();

			if (logic.getDef().isBillboard())
				transform= billboarded(transform);

			auto pos2d= util::Coord(transform.translation.xy(), logic.getCoordinateSpace()).
				converted(util::Coord::World).getValue();
			auto scale2d= util::Coord(transform.scale.xy(), logic.getScaleCoordinateSpace(), true).
				converted(util::Coord::World).getValue();

			transform.translation.x= pos2d.x;
			transform.translation.y= pos2d.y;
			transform.scale.x= scale2d.x;
			transform.scale.y= scale2d.y;
		}

		{ PROFILE();	
			if (logic.getDef().isSnappingToPixels()){
				util::Vec2d pixpos;

				// Scale to pixel-coordinates
				pixpos= util::Coord(
							transform.translation.xy(),
							util::Coord::World).
						converted(util::Coord::View_Pixels).getValue() + viewport;

				// Rounding to pixels
				pixpos= util::Vec2d(pixpos + util::Vec2d(0.5)).floored();
				auto pos2d= util::Coord(pixpos-viewport, util::Coord::View_Pixels).
						converted(util::Coord::World).getValue();

				transform.translation.x= pos2d.x;
				transform.translation.y= pos2d.y;
			}
			
			ensure(	std::isfinite(transform.translation.x) &&
					std::isfinite(transform.translation.y));
			
			re.transform= transform;
			re.visible= true;
			re.finalize();
		}
	};
	
	auto origin= gridPos((campos - rad).xyz());
	auto size= gridPos((campos + rad).xyz()).xy() - gridPos((campos - rad).xyz()).xy();
	size += Grid::Vec2(1);

	util::DynArray<const ModelEntityLogic*> logics= spatialGrid.coneQuery(
			origin,
			size,
			-camera.getPerspectiveMul());

	logics.append(viewSpace);

	util::DynArray<RenderEntity> result;
	for (SizeType i= 0; i < logics.size(); ++i){
		RenderEntity re= minimalRenderEntity(*logics[i]);
		visibility_update(re);
		
		if (re.visible)
			result.pushBack(re);
	}
	return result;
}

auto EntityMgr::RenderEntityCache::gridPos(util::Vec3d p) const -> Grid::Vec {
	return (p/chunkSize).discretized<Grid::Vec::Value>();
}

void EntityMgr::RenderEntityCache::addToGrid(const ModelEntityLogic& logic){
	ModelEntityLogic::BoundingBox bb= logic.getBoundingBox();
	Grid::Vec origin= gridPos(logic.getPosition() + bb.getMin().xyz());
	Grid::Vec end= gridPos(logic.getPosition() + bb.getMax().xyz()) + Grid::Vec(1);
	Grid::Vec size= end - origin;

	// Assuming that position lies inside boundings
	Grid::Vec pos= gridPos(logic.getTransform().translation);
	ensure(	pos.componentsGreaterOrEqualThan(origin) &&
			pos.componentsLessThan(origin + size));

	spatialGrid.add(&logic, origin, size);
}

//
// EntityMgr
//

void EntityMgr::calcLighting(const RenderFrameConfig& cfg){
	PROFILE();
	Framebuffer& casterBuf= cfg.camera->getShadowCasterBuf();

	casterBuf.bind();
	
	genericST.setCamera(*cfg.camera);
	genericST.setEnvLight(util::Color{0,0,0,0}, util::Vec2f{0,-1});
	genericST.setLightCount(0);
	
	{ PROFILE();
		// Draw shadow casting entities to buffer
		for (uint32 i=0; i<cfg.entities.size(); i++){
			if (!cfg.entities[i].def->isShadowCasting())
				continue;
			
			{ PROFILE();
				genericST.setEntity(*cfg.entities[i].def, *cfg.entities[i].logic);
				genericST.use();
				genericST.setTransform(cfg.entities[i].transform);

				cfg.entities[i].def->draw();
			}
		}
	}

	{ PROFILE();
		// Calculate ShadowCasterMaps
		hardware::gGlState->setClearColor({1, 0, 0, 1});
		for (uint32 i=0; i<cfg.lights.size(); i++){
			if (!cfg.lights[i]->getDef().hasShadows())
				continue;

			Framebuffer &buf= cfg.lights[i]->getShadowCasterMap();

			buf.bind();

			hardware::gGlState->clear(GL_COLOR_BUFFER_BIT);
			shadowCasterST.setLight(*cfg.lights[i]);

			for (uint32 i=0; i<cfg.entities.size(); i++){
				if (!cfg.entities[i].def->isShadowCasting())
					continue;

				shadowCasterST.setEntity(*cfg.entities[i].def);
				shadowCasterST.setTransform(cfg.entities[i].transform);
				shadowCasterST.use();

				cfg.entities[i].def->draw();
			}

		}
	}

	{ PROFILE();
		// Generate shadow maps
		for (uint32 i=0; i<cfg.lights.size(); i++){
			if (!cfg.lights[i]->getDef().hasShadows())
				continue;
			
			shadowMapST.generate(cfg.lights[i]->getShadowCasterMap().getTextureDId(),
								 cfg.lights[i]->getShadowMap());
		}
	}
}

bool EntityMgr::ReSortByDepth::operator()(const RenderEntity& a, const RenderEntity& b){
	const real32 depth_epsilon= 0.0001;

	if (a.layer == b.layer){
		if ( util::abs(a.depth - b.depth) <= depth_epsilon){
			if (a.priority < b.priority)
				return true;
		}
		else if (a.depth < b.depth){
			return true;
		}
	}
	else if (a.layer < b.layer){
		return true;
	}

	return false;
}

util::DynArray<EntityMgr::RenderEntity> EntityMgr::sortedByDepth(util::DynArray<RenderEntity> entities){
	PROFILE();
	// Sort by depth and drawPriority
	std::stable_sort(entities.begin(), entities.end(), ReSortByDepth());
	return entities;
}

void EntityMgr::processAnalysis(const RenderingAnalyzer::Analysis& a, const RenderFrameConfig& cfg) {
	PROFILE();
	
	entityToBatch.clear();
	
	// <batchContentHash>
	util::Set<uint32> preservedBatches;
	SizeType preserved_count= 0;
	
	for (const auto& a_batch : a.batches) {
		// Check if batch was same in the previous frame
		auto preserved_batch_it= batchMap.find(a_batch.contentHash);
		if (preserved_batch_it != batchMap.end()) {
			preservedBatches.insert(a_batch.contentHash);
			
			for (const auto& logic : a_batch.modelLogics) {
				entityToBatch[logic->getContentHash()]= &preserved_batch_it->second;
			}

			++preserved_count;
			continue;
		}

		// Create RenderBatch

		RenderBatch batch;
		batch.lastEntityLogic= a_batch.modelLogics.back();

		for (SizeType i= 0; i < a_batch.meshes.size(); ++i) {
			PROFILE();
			ensure(i < a_batch.modelLogics.size());
			const RenderEntity* re= cfg.logicToRenderEntity.find(a_batch.modelLogics[i])->second;
			ensure(re);

			TriMesh tempmesh= *a_batch.meshes[i];
			tempmesh.scale(re->transform.scale.casted<util::Vec3f>());
			tempmesh.rotate(re->transform.rotation.casted<util::Quatf>());
			tempmesh.translate(re->transform.translation.casted<util::Vec3f>());
			
			for (SizeType i= 0; i < tempmesh.getVertexCount(); ++i) {
				auto vtex= tempmesh.getVertex(i);
				/// @todo Do transforms here for fastness
				vtex.color *=
					re->def->getColorMul()*re->logic->getColorMul();
				tempmesh.setVertex(i, vtex);
			}

			batch.mesh.add(tempmesh);
		}

		ensure(a_batch.material);
		batch.model.setMaterial(*a_batch.material);
		batch.model.setMesh(batch.mesh);

		batch.def= a_batch.modelLogics.front()->getDef(); // Copy def
		batch.def.setColorMul(util::Color{}); // Mesh is colored
		batch.def.setModel(batch.model);

		batch.logic= *a_batch.modelLogics.front(); // Copy logic
		batch.logic.setColorMul(util::Color{}); // Mesh is colored
		batch.logic.setDef(batch.def);
		batch.logic.setTransform(EntityLogic::Transform()); // Reset transform
	
		if (preservedBatches.find(a_batch.contentHash) != preservedBatches.end())
			print(debug::Ch::Visual, debug::Vb::Moderate, "Batch hash collision: %u", a_batch.contentHash);

		// Add renderbatch to structures

		preservedBatches.insert(a_batch.contentHash);
		
		auto it= batchMap.insert(std::pair<uint32, RenderBatch>(a_batch.contentHash, std::move(batch))).first;
		RenderBatch* batch_ptr= &it->second;
		
		for (const auto& logic : a_batch.modelLogics){
			entityToBatch[logic->getContentHash()]= batch_ptr;
		}
	}
	
	//print(debug::Ch::Visual, debug::Vb::Trivial, "Preserved batch count: %zu", preserved_count);
	//print(debug::Ch::Visual, debug::Vb::Trivial, "New batch count: %zu", batchMap.size() - preserved_count);
	
	// Delete batches that weren't preserved
	for (auto it= batchMap.begin(); it != batchMap.end();){
		if (!preservedBatches.count(it->first)){
			it= batchMap.erase(it);
		}
		else {
			++it;
		}
	}
}

void EntityMgr::optimizeRenderFrameConfig(RenderFrameConfig& cfg){
	PROFILE();
	util::DynArray<RenderEntity> entities;	
	for (SizeType i= 0; i < cfg.entities.size(); ++i){
		// Check if entity belongs to a batch
		ensure(cfg.entities[i].logic);
		auto batch_it= entityToBatch.find(cfg.entities[i].logic->getContentHash());
		if (batch_it != entityToBatch.end()){
			ensure(batch_it->second);
			const RenderBatch& batch= *batch_it->second;
			// Use batch as a RenderEntity
			entities.pushBack(batch.asRenderEntity());

			// Jump over entities which belong to the same batch
			while (	i < cfg.entities.size() &&
					cfg.entities[i].logic != batch.lastEntityLogic) {
				++i;
			}

			continue;
		}
		else {
			entities.pushBack(cfg.entities[i]);
		}
	}

	cfg.entities= entities;
	cfg.finalize();
}

EntityMgr::RenderEntity EntityMgr::minimalRenderEntity(const ModelEntityLogic& logic){
	RenderEntity re= {};
	re.def= &logic.getDef();
	re.logic= &logic;
	return re;
}

} // visual
} // clover
