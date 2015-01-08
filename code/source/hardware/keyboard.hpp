#ifndef CLOVER_HARDWARE_KEYBOARD_HPP
#define CLOVER_HARDWARE_KEYBOARD_HPP

#include "build.hpp"
#include "util/string.hpp"

#include <GLFW/glfw3.h>

namespace clover {
namespace hardware {

class Device;

/// @todo Remove, replaced with hardware::KeyboardHidDevice
class Keyboard {
public:
	Keyboard();
	
	bool isDown(int32 code);
	bool isPressed(int32 code);
	bool isReleased(int32 code);

	/// @return Characters written in last frame
	util::Str8 getWrittenStr();
	uint32 getWrittenBackspaceCount();
	
protected:
	friend class Device;

	void update();
	void clearWriting();
	
	static const int32 keyCount= GLFW_KEY_LAST+1;
	int32 currentKBStates[keyCount];
	int32 previousKBStates[keyCount];

	static util::Str8 writtenStr;
	static uint32 backspaceCount;
	
	//static void GLFWCALL charCallback(int32 character, int32 action);
	//static void GLFWCALL keyCallback(int32 character, int32 action);
};

extern Keyboard *gKeyboard;

} // hardware
} // clover

#endif // CLOVER_HARDWARE_KEYBOARD_HPP
