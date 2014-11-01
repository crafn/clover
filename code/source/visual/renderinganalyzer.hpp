#ifndef CLOVER_VISUAL_RENDERINGANALYZER_HPP
#define CLOVER_VISUAL_RENDERINGANALYZER_HPP

#include "build.hpp"
#include "entity.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "entitylogic_model.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace visual {

class ModelEntityLogic;

/// Gives hints how to optimize rendering dynamically
class RenderingAnalyzer {
public:
	struct Analysis {
		struct Batch {
			uint32 contentHash; // Batches with same hash looks the same
			util::DynArray<const ModelEntityLogic*> modelLogics;
			util::DynArray<const TriMesh*> meshes;
			const Material* material= nullptr;
		};

		/// Meshes in these should be batched together
		util::DynArray<Batch> batches;
	};

	void onFrameStart(/*util::Vec2d worldview_center, util::Vec2d worldview_rad*/);
	void onDraw(const ModelEntityLogic& logic);
	Analysis analyze();

private:

	/// Necessary information for figuring out which meshes can be merged
	struct MeshInfo {
		MeshInfo(const visual::ModelEntityLogic& logic);

		const visual::ModelEntityLogic* entityLogic;
		Entity::Transform transform; // Transform of the mesh
		uint32 batchCompatibilityHash= 0;
		uint32 contentHash= 0; // MeshInfos with same contentHash looks the same
		bool unbatchable= false;
	};

	struct FrameInfo {
		util::DynArray<MeshInfo> meshInfos;
		util::Vec2d viewCenter, viewRadius;
	};

	static bool batchCompatible(const MeshInfo& m1, const MeshInfo& m2);
	static const TriMesh* asTriMesh(const BaseMesh* mesh);
	static uint32 hash(const BaseMesh* mesh);
	static uint32 hash(const Material* mat);


	FrameInfo frameInfo;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_RENDERINGANALYZER_HPP