#ifndef CLOVER_HARDWARE_DEVICE_HPP
#define CLOVER_HARDWARE_DEVICE_HPP

#include "build.hpp"
#include "util/math.hpp"
#include "util/unique_ptr.hpp"

struct GLFWwindow;

namespace clover {
namespace ui { namespace hid {

class HidMgr;

}} // ui::hid
namespace hardware {

/// Interface to devices and window
class Device {
public:
	Device();
	virtual ~Device();
	
	void create(util::Str8 title);
	
	static bool isBigEndian();

	real32 getFrameTime();
	void updateFrameTime();
	
	void updateEvents();
	void sleep(real32 d);

	void getViewportSize(int& x, int32 &y);
	util::Vec2i getViewportSize();

	//x/y
	real32 getAspectRatio() const { return aspectRatio; }

	/// @return vector with other component being 1.0 and other >= 1.0 in regards to aspect ratio
	/// @todo Shouldn't be here
	util::Vec2d getAspectVector() const { return aspectVector; }

	void clearBuffers();

	void setViewport();
	void setViewport(util::Vec2i v);

	void swapBuffers();
	
	ui::hid::HidMgr& getHidMgr() const { ensure(hidMgr); return *hidMgr; }
	
	/// Full path to working directory
	util::Str8 getWorkingDirectory() const;
	
	GLFWwindow& getWindow() const;

private:
	void updateAspectRatio();
	 
	void initGlfw();
	void initGlew();
	void createWindow(const util::Str8& title, util::Vec2i resolution, int32 multisamples, bool fullscreen);

	static void glfwErrorCallback(int32 error, const char* description);

	GLFWwindow* window;

	int32 viewportX, viewportY;
	real32 aspectRatio;
	util::Vec2d aspectVector;

	real32 prevFrameTime;
	real32 curFrameTime;
	
	using HidMgrPtr= util::UniquePtr<ui::hid::HidMgr>;
	HidMgrPtr hidMgr;
};

} // hardware
} // clover

#endif // CLOVER_HARDWARE_DEVICE_HPP
