#ifndef CLOVER_HARDWARE_HID_DEVICE_KEYBOARD_HPP
#define CLOVER_HARDWARE_HID_DEVICE_KEYBOARD_HPP

#include "build.hpp"
#include "ui/hid/controls/control_button.hpp"
#include "ui/hid/controls/control_text.hpp"
#include "ui/hid/device.hpp"

#include <array>
#include <GLFW/glfw3.h>

namespace clover {
namespace hardware {

/// Representation of physical keyboard device
class KeyboardHidDevice : public ui::hid::Device {
public:
	static Ptr create(const ConstructInfo& info){ return Ptr(new KeyboardHidDevice(info)); }
	virtual ~KeyboardHidDevice();

	virtual void update() override;
	virtual util::Str8 getTypeName() const override { return "Keyboard"; }
	
protected:
	KeyboardHidDevice(const ConstructInfo& info);

private:
	void createButtonControls();
	/// This is called when user writes something on keyboard.
	/// GLFW handles modifier keys etc. internally and gives correct characters.
	static void characterCallback(GLFWwindow* window, uint32 character);
	static void keyCallback(GLFWwindow* window, int32 key, int32 scancode, int32 action, int32 mod);
	
	/// Written text can only be read through static callback, so this must be also static
	static util::DynArray<ui::hid::TextControl*> textControls;
	
	ui::hid::TextControl& textControl;
	util::DynArray<ui::hid::ButtonControl*> keyControls;
	
	struct KeyDef { int32 glfwCode; const char* name; };
	/// List of all keys that can be used in input map cfg
	/// Keycodes represent physical key positions in US layout
	static constexpr KeyDef keyList[] = {
		{GLFW_KEY_SPACE,		 "space"},
		{GLFW_KEY_APOSTROPHE,	 "apostrophe"},
		{GLFW_KEY_COMMA,		 "comma"},
		{GLFW_KEY_MINUS,		 "minus"},
		{GLFW_KEY_PERIOD,		 "period"},
		{GLFW_KEY_SLASH,		 "slash"},
		{GLFW_KEY_0,			 "0"},
		{GLFW_KEY_1,			 "1"},
		{GLFW_KEY_2,			 "2"},
		{GLFW_KEY_3,			 "3"},
		{GLFW_KEY_4,			 "4"},
		{GLFW_KEY_5,			 "5"},
		{GLFW_KEY_6,			 "6"},
		{GLFW_KEY_7,			 "7"},
		{GLFW_KEY_8,			 "8"},
		{GLFW_KEY_9,			 "9"},
		{GLFW_KEY_SEMICOLON,	 "semicolon"},
		{GLFW_KEY_EQUAL,		 "equal"},
		{GLFW_KEY_A,			 "a"},
		{GLFW_KEY_B,			 "b"},
		{GLFW_KEY_C,			 "c"},
		{GLFW_KEY_D,			 "d"},
		{GLFW_KEY_E,			 "e"},
		{GLFW_KEY_F,			 "f"},
		{GLFW_KEY_G,			 "g"},
		{GLFW_KEY_H,			 "h"},
		{GLFW_KEY_I,			 "i"},
		{GLFW_KEY_J,			 "j"},
		{GLFW_KEY_K,			 "k"},
		{GLFW_KEY_L,			 "l"},
		{GLFW_KEY_M,			 "m"},
		{GLFW_KEY_N,			 "n"},
		{GLFW_KEY_O,			 "o"},
		{GLFW_KEY_P,			 "p"},
		{GLFW_KEY_Q,			 "q"},
		{GLFW_KEY_R,			 "r"},
		{GLFW_KEY_S,			 "s"},
		{GLFW_KEY_T,			 "t"},
		{GLFW_KEY_U,			 "u"},
		{GLFW_KEY_V,			 "v"},
		{GLFW_KEY_W,			 "w"},
		{GLFW_KEY_X,			 "x"},
		{GLFW_KEY_Y,			 "y"},
		{GLFW_KEY_Z,			 "z"},
		{GLFW_KEY_LEFT_BRACKET,	 "leftBracket"},
		{GLFW_KEY_BACKSLASH,	 "backslash"},
		{GLFW_KEY_RIGHT_BRACKET, "rightBracket"},
		{GLFW_KEY_GRAVE_ACCENT,	 "graveAccent"},
		{GLFW_KEY_WORLD_1,		 "world1"},
		{GLFW_KEY_WORLD_2,		 "world2"},
		{GLFW_KEY_ESCAPE,		 "escape"},
		{GLFW_KEY_ENTER,		 "enter"},
		{GLFW_KEY_TAB,			 "tab"},
		{GLFW_KEY_BACKSPACE,	 "backspace"},
		{GLFW_KEY_INSERT,		 "insert"},
		{GLFW_KEY_DELETE,		 "delete"},
		{GLFW_KEY_RIGHT,		 "right"},
		{GLFW_KEY_LEFT,			 "left"},
		{GLFW_KEY_DOWN,			 "down"},
		{GLFW_KEY_UP,			 "up"},
		{GLFW_KEY_PAGE_UP,		 "pageUp"},
		{GLFW_KEY_PAGE_DOWN,	 "pageDown"},
		{GLFW_KEY_HOME,			 "home"},
		{GLFW_KEY_END,			 "end"},
		{GLFW_KEY_CAPS_LOCK,	 "capsLock"},
		{GLFW_KEY_SCROLL_LOCK,	 "scrollLock"},
		{GLFW_KEY_NUM_LOCK,		 "numLock"},
		{GLFW_KEY_PRINT_SCREEN,	 "printScreen"},
		{GLFW_KEY_PAUSE,		 "pause"},
		{GLFW_KEY_F1,			 "f1"},
		{GLFW_KEY_F2,			 "f2"},
		{GLFW_KEY_F3,			 "f3"},
		{GLFW_KEY_F4,			 "f4"},
		{GLFW_KEY_F5,			 "f5"},
		{GLFW_KEY_F6,			 "f6"},
		{GLFW_KEY_F7,			 "f7"},
		{GLFW_KEY_F8,			 "f8"},
		{GLFW_KEY_F9,			 "f9"},
		{GLFW_KEY_F10,			 "f10"},
		{GLFW_KEY_F11,			 "f11"},
		{GLFW_KEY_F12,			 "f12"},
		{GLFW_KEY_F13,			 "f13"},
		{GLFW_KEY_F14,			 "f14"},
		{GLFW_KEY_F15,			 "f15"},
		{GLFW_KEY_F16,			 "f16"},
		{GLFW_KEY_F17,			 "f17"},
		{GLFW_KEY_F18,			 "f18"},
		{GLFW_KEY_F19,			 "f19"},
		{GLFW_KEY_F20,			 "f20"},
		{GLFW_KEY_F21,			 "f21"},
		{GLFW_KEY_F22,			 "f22"},
		{GLFW_KEY_F23,			 "f23"},
		{GLFW_KEY_F24,			 "f24"},
		{GLFW_KEY_F25,			 "f25"},
		{GLFW_KEY_KP_0,			 "kp0"},
		{GLFW_KEY_KP_1,			 "keypad1"},
		{GLFW_KEY_KP_2,			 "keypad2"},
		{GLFW_KEY_KP_3,			 "keypad3"},
		{GLFW_KEY_KP_4,			 "keypad4"},
		{GLFW_KEY_KP_5,			 "keypad5"},
		{GLFW_KEY_KP_6,			 "keypad6"},
		{GLFW_KEY_KP_7,			 "keypad7"},
		{GLFW_KEY_KP_8,			 "keypad8"},
		{GLFW_KEY_KP_9,			 "keypad9"},
		{GLFW_KEY_KP_DECIMAL,	 "keypadDecimal"},
		{GLFW_KEY_KP_DIVIDE,	 "keypadDivide"},
		{GLFW_KEY_KP_MULTIPLY,	 "keypadMultiply"},
		{GLFW_KEY_KP_SUBTRACT,	 "keypadSubtract"},
		{GLFW_KEY_KP_ADD,		 "keypadAdd"},
		{GLFW_KEY_KP_ENTER,		 "keypadEnter"},
		{GLFW_KEY_KP_EQUAL,		 "keypadEqual"},
		{GLFW_KEY_LEFT_SHIFT,	 "leftShift"},
		{GLFW_KEY_LEFT_CONTROL,	 "leftControl"},
		{GLFW_KEY_LEFT_ALT,		 "leftAlt"},
		{GLFW_KEY_LEFT_SUPER,	 "leftSuper"},
		{GLFW_KEY_RIGHT_SHIFT,	 "rightShift"},
		{GLFW_KEY_RIGHT_CONTROL, "rightControl"},
		{GLFW_KEY_RIGHT_ALT,	 "rightAlt"},
		{GLFW_KEY_RIGHT_SUPER,	 "rightSuper"},
		{GLFW_KEY_MENU,			 "menu"},
	};
	static constexpr SizeType keyCount= sizeof(keyList)/sizeof(*keyList);
};

} // hardware
} // clover

#endif // CLOVER_HARDWARE_HID_DEVICE_KEYBOARD_HPP
