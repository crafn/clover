#ifndef CLOVER_HARDWARE_MOUSE_HPP#define CLOVER_HARDWARE_MOUSE_HPP#include "build.hpp"#include "util/coord.hpp"#include <GL/glew.h>#include <GLFW/glfw3.h>namespace clover {namespace hardware {class Device;/// @todo Remove, replaced with hardware::MouseHidDeviceclass Mouse {public:	Mouse();	util::Coord getPosition() const;	void setPosition(util::Coord c);	/// Works regardless of lock()ing	util::Coord getDifference() const;	bool limit(const util::Vec2i& min, const util::Vec2i& max);	void hide(bool b= true);	bool isHided();	/// Locks position	void lock(bool b=true);	bool isDown(uint32 btn) const;	bool isReleased(uint32 btn) const;	bool isPressed(uint32 btn) const;	int32 getWheelTurn() const;protected:	friend class Device;	void update();	void setRawPosition(const util::Vec2i& p);	util::Coord convertRawPos(const util::Vec2i& p);	  util::Vec2i rawPosition;	util::Coord prevPosition;	util::Coord position;	util::Coord dif;
	static const int32 buttonCount= GLFW_MOUSE_BUTTON_LAST +1;
	bool curStates[buttonCount];
	bool prevStates[buttonCount];

	int32 curWheelState;
	int32 prevWheelState;	bool hided;	bool locked;};// Device hoitaaextern Mouse* gMouse;} // hardware} // clover#endif // CLOVER_HARDWARE_MOUSE_HPP