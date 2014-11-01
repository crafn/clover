#include "device.hpp"
#include "audio/audiodevice.hpp"
#include "audio/audiodevice_dummy.hpp"
#include "debug/debugprint.hpp"
#include "hardware/audiodevice_pa.hpp"
#include "hardware/clstate.hpp"
#include "hardware/glstate.hpp"
#include "hardware/mouse.hpp"
#include "hardware/keyboard.hpp"
#include "global/cfg_mgr.hpp"
#include "ui/hid/hid_mgr.hpp"
#include "global/exception.hpp"
#include "util/math.hpp"
#include "util/misc.hpp"
#include "util/string.hpp"
#include "util/time.hpp"
#include "util/filewatcher.hpp"
#include "util/map.hpp"

#if OS == OS_WINDOWS
    #include <windows.h>
    #include <winsock.h>
#endif

#if OS == OS_LINUX
    #include <arpa/inet.h>
#endif

#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <chrono>
#include <stdexcept>
#include <sstream>
#include <thread>

namespace clover {
namespace hardware {

Device* gDevice;

Device::Device()
		: window(nullptr){
}

Device::~Device(){
	delete audio::gAudioDevice;
	
	hidMgr.reset();
	
	util::FileWatcher::globalShutdown();

	delete hardware::gMouse;
	hardware::gMouse= nullptr;

	delete hardware::gKeyboard;
	hardware::gKeyboard= nullptr;
	
    delete hardware::gGlState;
    hardware::gGlState= nullptr;
	
	delete hardware::gClState;
	hardware::gClState= nullptr;
	
	glfwDestroyWindow(window);
	window= nullptr;
	glfwTerminate();
	
    print(debug::Ch::Device, debug::Vb::Moderate, "Device shutdown succeeded");
}

void Device::create(util::Str8 title){
	initGlfw();

    createWindow(	title,
					global::gCfgMgr->get<util::Vec2i>("hardware::resolution", util::Vec2i{800, 600}),
					global::gCfgMgr->get<int32>("hardware::multisamples", 0),
					global::gCfgMgr->get<bool>("hardware::fullscreen", false));


	initGlew();
	
	// Initialize "hardware" subsystems
	
	hardware::gGlState= new hardware::GlState();
    hardware::gGlState->errorCheck("GLError after glewInit()");
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);

	hardware::gClState= new hardware::ClState();
	hardware::gClState->choosePlatformWiselyAndCreateContextNicely();

	hardware::gMouse= new Mouse();
	hardware::gKeyboard= new Keyboard();
	
	util::FileWatcher::globalInit();
	
	hidMgr= HidMgrPtr(new ui::hid::HidMgr());
	hidMgr->create();
	
	if (global::gCfgMgr->get<bool>("hardware::enableAudio", true))
		new hardware::PaAudioDevice();
	else
		new audio::DummyAudioDevice();
	
    prevFrameTime= 0.0;
    curFrameTime= 0.0;
}

bool Device::isBigEndian(){
    if (htonl(1)==1)return true;
    return false;
}

void Device::updateFrameTime(){
    prevFrameTime= curFrameTime;
    curFrameTime= glfwGetTime();
}

real32 Device::getFrameTime(){
    if(curFrameTime-prevFrameTime < 0.00001)
        return 0.00001;

    return curFrameTime-prevFrameTime;
}

void Device::updateEvents(){
	util::Timer::nextFrames();
	
	glfwPollEvents();
	
	hardware::gKeyboard->update();
	hardware::gMouse->update();

	if (glfwGetWindowAttrib(window, GLFW_VISIBLE)){
		// Detect change of window size
		int32 width, height;
		glfwGetWindowSize(window, &width, &height);

		// Make life easier in a place somewhere else by not allowing zero sizes for viewport
		if (width <= 0)
			width= 1;
		if (height <= 0)
			height= 1;

		if (width != viewportX || height != viewportY){
			viewportX= width;
			viewportY= height;

			updateAspectRatio();
		}

	}
	hidMgr->update();
	
	util::FileWatcher::launchCallbacks();
}

void Device::sleep(real32 d){
	std::chrono::milliseconds duration((int)(d*1000.0f));
    std::this_thread::sleep_for(duration);
}

void Device::getViewportSize(int& x, int32 &y){
    x= viewportX;
    y= viewportY;
}

util::Vec2i Device::getViewportSize(){
	ensure(viewportX != 0 && viewportY != 0);
    return {viewportX, viewportY};
}

void Device::clearBuffers(){
    //glClearColor(0.7, 0.7, 0.7, 0.0);
    //glClear(GL_COLOR_BUFFER_BIT);
}

void Device::setViewport(){

    glViewport(0, 0, viewportX, viewportY);

}

void Device::setViewport(util::Vec2i v){
    glViewport(0,0,v.x,v.y);
}

void Device::swapBuffers(){
	#ifndef RELEASE
	hardware::gGlState->errorCheck("Before SwapBuffers");
	#endif

    //glFinish();
    glfwSwapBuffers(window);
}

util::Str8 Device::getWorkingDirectory() const {
	return util::Str8(boost::filesystem::path(boost::filesystem::current_path()).string()) + "/";
}

GLFWwindow& Device::getWindow() const {
	ensure(window);
	return *window;
}

void Device::updateAspectRatio(){
	aspectRatio= (real64)viewportX/viewportY;

	real64 aspect[2]= {1.,1.};
	if (aspectRatio > 1.0)
		aspect[1]=aspectRatio;
	else aspect[0]=1.0/aspectRatio;

	aspectVector= util::Vec2d{aspect[0], aspect[1]};

	setViewport();
}

void Device::initGlfw(){
	glfwSetErrorCallback(glfwErrorCallback);

	if (glfwInit() != GL_TRUE)
        throw(global::Exception("GLFW init failed"));
	
	int32 glfw_maj, glfw_min, glfw_rev;
	glfwGetVersion(&glfw_maj, &glfw_min, &glfw_rev);
	
	print(debug::Ch::Device, debug::Vb::Trivial, "GLFW version: %i.%i.%i", glfw_maj, glfw_min, glfw_rev);
}

void Device::initGlew(){
	glewExperimental= GL_TRUE;
    GLenum err = glewInit();

    #if OS == OS_WINDOWS
/*
        //Pitää määritellä käsin koska glew ei osaa
        //Jaha. glewExperimental= GL_TRUE toimii kans

        glGenVertexArrays= (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
        glBindVertexArray= (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
        glDeleteVertexArrays= (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");

        glGenFramebuffers= (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
        glBindFramebuffer= (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
        glFramebufferTexture2D= (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
        glCheckFramebufferStatus= (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
        glDeleteFramebuffers= (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");

        glGenerateMipmap= (PFNGLGENERATEMIPMAPEXTPROC)wglGetProcAddress("glGenerateMipmapEXT");*/

    #endif

    if (err != GLEW_OK)
        throw global::Exception("GLEW init failed");
    
    print(debug::Ch::OpenGL, debug::Vb::Trivial, "GLEW version: %s", glewGetString(GLEW_VERSION));
}

void Device::createWindow(const util::Str8& title, util::Vec2i resolution, int32 multisamples, bool fullscreen){
	glfwWindowHint(GLFW_SAMPLES, multisamples);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	GLFWmonitor* monitor= nullptr; // Null monitor == windowed mode
	
	if (fullscreen){
		monitor= glfwGetPrimaryMonitor();
		if (!monitor)
			throw global::Exception("Primary monitor not found");
	}

    window= glfwCreateWindow(	resolution.x, resolution.y,
								title.cStr(),
								monitor,
								nullptr);
				
	if (!window)
        throw global::Exception("Window creation failed");
	
	glfwMakeContextCurrent(window);
 
	int32 ma, mi;
	ma= glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
	mi= glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
	print(debug::Ch::OpenGL, debug::Vb::Trivial, "OpenGL version: %i.%i", ma, mi);

    viewportX= resolution.x;
    viewportY= resolution.y;

    updateAspectRatio();
	
	// VSync
	glfwSwapInterval(0);
	glfwSwapBuffers(window);
}

void Device::glfwErrorCallback(int32 error, const char* description){
	print(debug::Ch::Device, debug::Vb::Critical, "Glfw error: %s", description);
}

} // hardware
} // clover