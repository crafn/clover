#include "cursor.hpp"
#include "ui/userinput.hpp"
#include "resources/cache.hpp"

// Debug
#include "element.hpp"

namespace clover {
namespace gui {

Cursor::Cursor()
		: touchElement(nullptr)
		, touchLockElement(nullptr)
		, underElement(nullptr)
		, dragElement(nullptr)
		, hides(0)
		, positionListener("host", "guiCursor", "cursorPosition",
				[this] (const util::Vec2d& p){
					mode= Mode::Mouse;
					updatePosition(p);
				})
		, deltaListener("host", "guiCursor", "cursorDeltaPosition",
				[this] (const util::Vec2d& d){
					mode= Mode::Mouse;
					delta= d;
				})
		, moveListener("host", "guiCursor", "cursorMove",
				[this] (const util::Vec2d& dir){
					mode= Mode::Analog;
					drift= dir;
				})
		, pushHideListener("host", "guiCursor", "pushHideCursor", [this] (){ ++hides; })
		, popHideListener("host", "guiCursor", "popHideCursor", [this] (){ --hides; }){
	cursorVisual.setDef("cursor_default");
	cursorVisual.setLayer(visual::EntityLogic::Layer::Gui);
	cursorVisual.setScale(util::Vec3d({20, 20, 1}));
	cursorVisual.setDrawPriority(999999);
	cursorVisual.setCoordinateSpace(util::Coord::View_Pixels);
}

void Cursor::preUpdate(){
	if (mode == Mode::Analog){
		const double dt= global::g_env->realClock->getDeltaTime();
		const double sensitivity= 2.0;
		util::Vec2d dif= drift*dt*sensitivity;

		auto new_pos= position + dif;
		new_pos.x= util::limited(new_pos.x, -1.0, 1.0);
		new_pos.y= util::limited(new_pos.y, -1.0, 1.0);
		updatePosition(new_pos);

		delta= dif;
	}

	util::Vec2d pos= getPosition().getValue()*global::g_env->device->getViewportSize()*0.5 + util::Vec2d({19,-19});
	cursorVisual.setPosition(pos.xyz());

	touchElement= touchLockElement;
	underElement= nullptr;
}


void Cursor::postUpdate(){
	ensure(hides >= 0);
	cursorVisual.setActive(hides == 0);
	
	if (gUserInput->isTriggered(UserInput::GuiStopDragging)){
		if (dragElement)
			dragElement->onStopDragging();
		dragElement= 0;
	}
}

util::Coord Cursor::getDelta(){
	auto c= util::Coord::VSt(delta);
	c.setRelative();
	return c;
}

void Cursor::touch(Element& e){
	if (touchElement && &e != touchElement){
		print(debug::Ch::Gui, debug::Vb::Critical, "GuiCursor::touch(..): trying to touch another element: %i", e.getType());
		return;
	}
	touchElement= &e;
}

void Cursor::startDragging(Element& e){
	if (dragElement && &e != dragElement){
		print(debug::Ch::Gui, debug::Vb::Critical, "GuiCursor::startDragging(..): trying to drag another element: %i", e.getType());
		return;
	}
	dragElement= &e;
}


bool Cursor::canTouchElement(Element& e){
	if (touchLockElement && &e != touchLockElement) return false;

	if (dragElement != 0 && dragElement != &e) return false;

	if (touchElement == &e || touchElement == 0){
		return true;			
	}

	return false;
}

bool Cursor::canStartDraggingElement(Element& e){
	if (touchLockElement == &e && dragElement == 0) return true;

	if (dragElement == 0) return true;

	return false;
}


void Cursor::touchLock(Element& e){
	touchLockElement= &e;
}

void Cursor::touchUnlock(){
	touchLockElement= 0;
}

} // gui
} // clover
