#ifndef CLOVER_GUI_CURSOR_HPP
#define CLOVER_GUI_CURSOR_HPP

#include "build.hpp"
#include "ui/hid/actionlistener.hpp"
#include "visual/entity.hpp"
#include "visual/entity_def_model.hpp"

namespace clover {
namespace gui {

class Element;

/// Cursor shown on screen
class Cursor {
public:
	Cursor();
	
	void preUpdate();
	
	virtual void postUpdate();
	
	util::Coord getPosition(){ return util::Coord::VSt(position); }
	util::Coord getDelta();

	/// Element calls
	void touch(Element& e);
	void stopTouching(){ touchElement= nullptr; }
	
	void startDragging(Element& e);
	void stopDragging(){ dragElement= nullptr; }

	void touchLock(Element& e);
	void touchUnlock();
	
	bool canTouchElement(Element& e);
	bool canStartDraggingElement(Element& e);
	
	bool canSetUnderElement(Element& e){ return underElement == nullptr; }
	void setUnderElement(Element& e){ underElement= &e; }
	Element* getUnderElement(){ return underElement; }
	
	Element* getTouchedElement(){ return touchElement; }
	Element* getDraggedElement(){ return dragElement; }
	
private:
	void updatePosition(const util::Vec2d& p){ position= p; }
	
	enum class Mode {
		Mouse,
		Analog
	};

	Mode mode= Mode::Mouse;

	util::Vec2d position;
	/// Movement from previous frame (locking ignored)
	util::Vec2d delta;
	/// Causes cursor to move when using analog controls
	util::Vec2d drift;

	Element *touchElement;
	Element *touchLockElement;
	Element* underElement;
	Element *dragElement;

	visual::Entity cursorVisual;
	
	int32 hides;
	
	ui::hid::ActionListener<nodes::SignalType::Vec2> positionListener;
	ui::hid::ActionListener<nodes::SignalType::Vec2> deltaListener;
	ui::hid::ActionListener<nodes::SignalType::Vec2> moveListener;
	ui::hid::ActionListener<> pushHideListener;
	ui::hid::ActionListener<> popHideListener;
};

} // gui
} // cursor

#endif // CLOVER_GUI_CURSOR_HPP
