#include "camera_mgr.hpp"
#include "global/event.hpp"
#include "global/event_mgr.hpp"
#include "camera.hpp"
#include "hardware/device.hpp"

namespace clover {
namespace visual {

CameraMgr::CameraMgr():
			selectedCamera(0){
	listenForEvent(global::Event::OnCameraCreate);
	listenForEvent(global::Event::OnCameraSelect);
	listenForEvent(global::Event::OnCameraDestroy);
}

const util::DynArray<Camera*>& CameraMgr::getCameras(){
	return cameras;
}

const util::DynArray<Camera*>& CameraMgr::getActiveCameras(){
	return activeCameras;
}

Camera& CameraMgr::getSelectedCamera(){
	ensure(selectedCamera);
	return *selectedCamera;
}


void CameraMgr::update(){
	for (auto m : activeCameras){
		m->setResolution(global::g_env.device->getViewportSize());
		m->update();
	}
}

void CameraMgr::onCameraSelect(Camera& c){
	selectedCamera= &c;
}

void CameraMgr::onCameraAdd(Camera& c){
	cameras.pushBack(&c);
}
void CameraMgr::onCameraRemove(Camera& c){
	auto it= cameras.find(&c);
	ensure(it != cameras.end());
	cameras.erase(it);
	
	if (selectedCamera == &c)selectedCamera= 0;
}

void CameraMgr::onCameraActivate(Camera& c){
	activeCameras.pushBack(&c);
}
void CameraMgr::onCameraDeactivate(Camera& c){
	auto it= activeCameras.find(&c);
	ensure(it != activeCameras.end());
	activeCameras.erase(it);
}

void CameraMgr::onEvent(global::Event& e){
	switch(e.getType()){
		case global::Event::OnCameraCreate:
			onCameraAdd(*e(global::Event::Object).getPtr<Camera>());
			if (e(global::Event::Active).getI())
				onCameraActivate(*e(global::Event::Object).getPtr<Camera>());
		break;
		
		case global::Event::OnCameraDestroy:
			onCameraRemove(*e(global::Event::Object).getPtr<Camera>());
		break;
		
		case global::Event::OnCameraSelect:
			onCameraSelect(*e(global::Event::Object).getPtr<Camera>());
		break;
	
		default:;
	}
}

} // visual
} // clover