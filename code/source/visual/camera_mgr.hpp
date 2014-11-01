#ifndef CLOVER_VISUAL_CAMERA_MGR_HPP
#define CLOVER_VISUAL_CAMERA_MGR_HPP

#include "build.hpp"
#include "global/eventreceiver.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace visual {

class Camera;

class CameraMgr : public global::EventReceiver {
public:
	CameraMgr();
	Camera& getSelectedCamera();

	const util::DynArray<Camera*>& getCameras();
	const util::DynArray<Camera*>& getActiveCameras();

	void update();
	void onEvent(global::Event&);

private:
	util::DynArray<Camera*> cameras;
	util::DynArray<Camera*> activeCameras;
	Camera* selectedCamera;

	friend class Camera;

	void onCameraAdd(Camera& c);
	void onCameraRemove(Camera& c);

	void onCameraActivate(Camera& c);
	void onCameraDeactivate(Camera& c);

	void onCameraSelect(Camera& c);
};

} // visual
} // clover

#endif // CLOVER_VISUAL_CAMERA_MGR_HPP