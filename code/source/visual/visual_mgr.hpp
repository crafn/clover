#ifndef CLOVER_VISUAL_MGR_HPP
#define CLOVER_VISUAL_MGR_HPP

#include "build.hpp"

namespace clover {
namespace visual {
	
class EntityMgr;
class CameraMgr;
class ParticleMgr;

/// Ylin graffamanageri
class VisualMgr {
public:

	VisualMgr();
	virtual ~VisualMgr();
	
	void renderFrame();
	
	EntityMgr& getEntityMgr(){ return *entityMgr; }
	CameraMgr& getCameraMgr(){ return *cameraMgr; }
	ParticleMgr& getParticleMgr(){ return *particleMgr; }

private:
	EntityMgr* entityMgr;
	CameraMgr* cameraMgr;
	ParticleMgr* particleMgr;
};


extern VisualMgr* gVisualMgr;

} // visual
} // clover

#endif // CLOVER_VISUAL_MGR_HPP