#include "mouse.hpp"#include "hardware/device.hpp"#include "util/math.hpp"#include "ui/userinput.hpp"#include <GLFW/glfw3.h>namespace clover {namespace hardware {Mouse* gMouse=0;Mouse::Mouse(){	   for (int32 i=0; i<buttonCount; i++){
		curStates[i]= false;
		prevStates[i]= false;
	}
	curWheelState= 0;
	prevWheelState= 0;	hided= false;	locked= false;	position.setType(util::Coord::View_Stretch);	prevPosition.setType(util::Coord::View_Stretch);}util::Coord Mouse::getPosition() const{	return gUserInput->getCursorPosition();
	return position;}util::Coord Mouse::getDifference() const{	return gUserInput->getCursorDifference();	return dif;}bool Mouse::limit(const util::Vec2i& min, const util::Vec2i& max){	bool changed= false;	if (rawPosition.x <= min.x){
		rawPosition.x= min.x; changed= true;
	}
	else if (rawPosition.x >= max.x){
		rawPosition.x= max.x; changed= true;
	}
	if (rawPosition.y <= min.x ){
		rawPosition.y= min.x; changed= true;
	}
	else if (rawPosition.y >= max.y){
		rawPosition.y= max.y; changed= true;
	}	if (changed){		setRawPosition(rawPosition);	}	return changed;}void Mouse::setRawPosition(const util::Vec2i& rawpos){	//glfwSetMousePos(rawpos.x, rawpos.y);	rawPosition= rawpos;	position= convertRawPos(rawpos);}void Mouse::hide(bool b){	hided= b;	if (hided){		//glfwDisable( GLFW_MOUSE_CURSOR );		setRawPosition(rawPosition); // Bugikorjauksia odotellessa (glfw 2.7.5)	}	else {		//glfwEnable( GLFW_MOUSE_CURSOR );		setRawPosition(rawPosition);	}}bool Mouse::isHided(){	return hided;}void Mouse::lock(bool b){	locked= b;}bool Mouse::isDown(uint32 btn) const {	if (btn == GLFW_MOUSE_BUTTON_LEFT)		return gUserInput->isTriggered(UserInput::MouseLeftDown);	if (btn == GLFW_MOUSE_BUTTON_MIDDLE)		return gUserInput->isTriggered(UserInput::MouseMiddleDown);	if (btn == GLFW_MOUSE_BUTTON_RIGHT)		return gUserInput->isTriggered(UserInput::MouseRightDown);	return false;	if (curStates[btn])return true;	return false;}bool Mouse::isReleased(uint32 btn) const {	if (btn == GLFW_MOUSE_BUTTON_LEFT)		return gUserInput->isTriggered(UserInput::MouseLeftRelease);	if (btn == GLFW_MOUSE_BUTTON_MIDDLE)		return gUserInput->isTriggered(UserInput::MouseMiddleRelease);	if (btn == GLFW_MOUSE_BUTTON_RIGHT)		return gUserInput->isTriggered(UserInput::MouseRightRelease);	return false;	if (!curStates[btn] && prevStates[btn]) return true;	return false;}bool Mouse::isPressed(uint32 btn) const {	if (btn == GLFW_MOUSE_BUTTON_LEFT)		return gUserInput->isTriggered(UserInput::MouseLeftPress);	if (btn == GLFW_MOUSE_BUTTON_MIDDLE)		return gUserInput->isTriggered(UserInput::MouseMiddlePress);	if (btn == GLFW_MOUSE_BUTTON_RIGHT)		return gUserInput->isTriggered(UserInput::MouseRightPress);		return false;		if (curStates[btn] && !prevStates[btn]) return true;	return false;}int32 Mouse::getWheelTurn() const {
	return curWheelState-prevWheelState;
}void Mouse::update(){		prevPosition= position;	util::Vec2i prev_raw_pos= rawPosition;
	//glfwGetMousePos(&rawPosition.x, &rawPosition.y);
	for (int32 i=0; i<Mouse::buttonCount; i++){
		//gMouse->prevStates[i]= gMouse->curStates[i];
		//gMouse->curStates[i]= glfwGetMouseButton(i);
	}
		position= convertRawPos(rawPosition);	dif= position-prevPosition;	// Ä-ä-ää	if (locked){		setRawPosition(prev_raw_pos);	}
	prevWheelState= curWheelState;

	//curWheelState= glfwGetMouseWheel();}util::Coord Mouse::convertRawPos(const util::Vec2i& p){	util::Vec2i viewport= global::g_env.device->getViewportSize();
	util::Coord view_stretch(util::Coord::View_Stretch);	view_stretch.setValue(util::Vec2d{	(p.x - (real64)viewport.x/2.0 )/((real64)viewport.x/2.0),									(-p.y + (real64)viewport.y/2.0)/((real64)viewport.y/2.0) });	return view_stretch;}} // hardware} // clover