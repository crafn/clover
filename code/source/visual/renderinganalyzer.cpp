#include "renderinganalyzer.hpp"
#include "entitylogic_model.hpp"
#include "global/cfg_mgr.hpp"

namespace clover {
namespace visual {

void RenderingAnalyzer::onFrameStart(/*util::Vec2d worldview_center, util::Vec2d worldview_rad*/){
	frameInfo.meshInfos.clear();
	//frameInfo.viewCenter= worldview_center;
	//frameInfo.viewRadius= worldview_rad;
}

void RenderingAnalyzer::onDraw(const ModelEntityLogic& logic){
	// Only trimesh entities should be added (discard particles)
	if (logic.getDef().getShadingType() != visual::ModelEntityDef::Shading_Generic ||
		logic.getDef().getModel() == nullptr ||
		logic.getDef().getModel()->getMesh() == nullptr ||
		logic.getDef().getModel()->getMaterial() == nullptr ||
		logic.getCoordinateSpace() != util::Coord::World) // Only world entities supported
		return;
	
	frameInfo.meshInfos.emplaceBack(logic);
}

RenderingAnalyzer::Analysis RenderingAnalyzer::analyze(){
	Analysis a;
	
	// Batch analysis
	
	SizeType group_begin_i= 0;
	SizeType drawable_count= 0;
	const int32 min_group_size= global::gCfgMgr->get<int32>("visual::minBatchEntityCount", 10);
	const int32 max_group_size= global::gCfgMgr->get<int32>("visual::maxBatchEntityCount", 100);
	const auto& mesh_infos= frameInfo.meshInfos;
	
	if (mesh_infos.empty() || min_group_size > max_group_size)
		return a;
	
	for (SizeType i= 0; i < mesh_infos.size(); ++i){
		int32 group_size= i - group_begin_i;
		
		// If this mesh_infos[i] is last of a group
		if (i + 1 == frameInfo.meshInfos.size() ||
			!batchCompatible(mesh_infos[i], mesh_infos[i + 1]) ||
			group_size == max_group_size ||
			mesh_infos[i + 1].unbatchable){
			
			if (group_size >= min_group_size){
				
				a.batches.emplaceBack();
				Analysis::Batch& b= a.batches.back();
				
				for (SizeType i2= group_begin_i; i2 < i + 1; ++i2){
					ensure(i2 < mesh_infos.size());
					
					auto& mesh_info= mesh_infos[i2];
					
					ensure(mesh_info.entityLogic);
					ensure(mesh_info.entityLogic->getDef().getModel());

					if (i2 == group_begin_i){
						// First round
						b.material= mesh_info.entityLogic->getDef().getModel()->getMaterial();
					}

					b.modelLogics.pushBack(mesh_info.entityLogic);
					b.meshes.pushBack(asTriMesh(mesh_info.entityLogic->getDef().getModel()->getMesh()));
					b.contentHash += mesh_info.contentHash;
				}
				drawable_count -= b.modelLogics.size() - 1;
			}
			group_begin_i= i + 1;
		}
		++drawable_count;
	}
	
	real64 ratio= (real64)drawable_count/mesh_infos.size();
	//print(debug::Ch::General, debug::Vb::Trivial, "Batch count: %zu", a.batches.size());
	//print(debug::Ch::General, debug::Vb::Trivial, "Optimized drawcount ratio: %f", ratio);
	return a;
}

RenderingAnalyzer::MeshInfo::MeshInfo(const visual::ModelEntityLogic& logic){
	entityLogic= &logic;
	contentHash= logic.getContentHash();
	batchCompatibilityHash= logic.getBatchCompatibilityHash();

	// Batching normalmapped things could be done using some uv tricks
	unbatchable=	logic.getDef().getModel()->getMaterial()->hasTexture(Material::TexType_Normal) || 
					logic.getDef().getModel()->getMaterial()->hasTexture(Material::TexType_EnvShadow); 
}

bool RenderingAnalyzer::batchCompatible(const MeshInfo& m1, const MeshInfo& m2){
	return m1.batchCompatibilityHash == m2.batchCompatibilityHash;
}

const TriMesh* RenderingAnalyzer::asTriMesh(const BaseMesh* mesh){
	// Only TriMeshes are batched
	debug_ensure(dynamic_cast<const TriMesh*>(mesh) != nullptr || mesh == nullptr);
	return static_cast<const TriMesh*>(mesh);
}

uint32 RenderingAnalyzer::hash(const BaseMesh* mesh){
	if (mesh)
		return mesh->getContentHash();
	else
		return 0;
}

uint32 RenderingAnalyzer::hash(const Material* mat){
	if (mat)
		return mat->getContentHash();
	else
		return 0;		
}

} // visual
} // clover
