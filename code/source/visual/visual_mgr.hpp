#ifndef CLOVER_VISUAL_MGR_HPP
#define CLOVER_VISUAL_MGR_HPP

#include "build.hpp"
#include "util/class_preproc.hpp"
#include "util/mem_chunk.hpp"

namespace clover {
namespace visual {
	
class ShaderMgr;
class EntityMgr;
class CameraMgr;
class ParticleMgr;

class VisualMgr {
public:

	VisualMgr();
	DELETE_MOVE(VisualMgr);
	DELETE_COPY(VisualMgr);
	virtual ~VisualMgr();

	void renderFrame();

	ShaderMgr& getShaderMgr(){ return *shaderMgr; }
	EntityMgr& getEntityMgr(){ return *entityMgr; }
	CameraMgr& getCameraMgr(){ return *cameraMgr; }
	ParticleMgr& getParticleMgr(){ return *particleMgr; }

private:
	ShaderMgr* shaderMgr;
	EntityMgr* entityMgr;
	CameraMgr* cameraMgr;
	ParticleMgr* particleMgr;

	//util::MemChunk modelDefMem;
	//util::MemChunk modelLogicMem;
};

extern VisualMgr* gVisualMgr;

} // visual
} // clover

#endif // CLOVER_VISUAL_MGR_HPP
