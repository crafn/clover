#include "build.hpp"
#if OS == OS_WINDOWS
	// need to include this before clstate.hpp, otherwise include order
	// goes wrong and this doesn't compile in mingw
	#include <GL/glew.h>
	#include <GL/wglew.h>
	#include <wingdi.h>
#endif

#include "hardware/clstate.hpp"

#include "debug/debugprint.hpp"
#include "global/cfg_mgr.hpp"
#include "glstate.hpp"
#include "global/file.hpp"

// Some of OpenCL 1.0 functions are deprecated
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <CL/cl_gl.h>

#if OS == OS_LINUX
	#include <GL/glx.h>
#endif

namespace clover {
namespace hardware {

ClState* gClState= nullptr;

ClState::ClState(){
	platform.id= 0;
	device.id= 0;
	context.id= 0;
}


void ClState::choosePlatformWiselyAndCreateContextNicely(){
	platform.id= 0;
	device.id= 0;
	context.id= 0;
	
	util::DynArray<util::Str8> preferred;
	
	util::Str8 user_preference= global::gCfgMgr->get<util::Str8>("hardware::openCLPlatform", "");
	if (!user_preference.empty())
		preferred.pushBack(user_preference);
	
	preferred.pushBack("NVIDIA");
	preferred.pushBack("NVIDIA CUDA");
	preferred.pushBack("Intel(R) OpenCL");
	preferred.pushBack("AMD Accelerated Parallel Processing");
	
	util::DynArray<Platform> platforms= queryPlatforms();
	print(debug::Ch::OpenCL, debug::Vb::Trivial, "Available OpenCL platforms (%lu):", (unsigned long)platforms.size());
	
	if (platforms.empty())
		throw global::Exception("No OpenCL platforms detected");
	
	for (auto &m : platforms){
		print(debug::Ch::OpenCL, debug::Vb::Trivial, "	  %s", m.name.cStr());
	}
	
	while(!platforms.empty()){
		
		platform.id= 0;
		
		// Valitse platform
		
		for (auto &m : preferred){
			
			auto it= 
				std::find_if(platforms.begin(), platforms.end(),
					[&m](Platform& p){ 
						if(p.name.lowerCased() == m.lowerCased())
							return true;
						return false;
					}
				);
			
			if (it != platforms.end()){
				print(debug::Ch::OpenCL, debug::Vb::Trivial, "-> %s", m.cStr());
				platform= *it;
				break;
			}
		}

		// Preferred not found, choose first platform
		if (platform.id == 0)
			platform= platforms.front();
		
		
		// Choose device
		
		util::DynArray<Device> devices= queryPlatformDevices(platform);
		
		print(debug::Ch::OpenCL, debug::Vb::Trivial, "Available OpenCL devices (%lu):", (unsigned long)devices.size());
		for (auto &m : devices){
			print(debug::Ch::OpenCL, debug::Vb::Trivial, "	  %s, Type: %i, isDefault: %i", m.name.cStr(), m.type, m.isDefault);
		}

		for (uint32 phase= 0; phase<4; ++phase){
			
			auto it= std::find_if(devices.begin(), devices.end(),
							[phase](Device& d){
								switch(phase){
									case 0: return d.isDefault;
									case 1: return d.type == Device::Gpu;
									case 2: return d.type == Device::Cpu;
									default: return true;
								}
								
							}
						);
			
			if (it != devices.end()){
				device= *it;
				print(debug::Ch::OpenCL, debug::Vb::Trivial, "-> %s", device.name.cStr());
				context= createContext(device);
				return;
			}
		
		}
		
		if (!preferred.empty()){
			// Try next preference
			preferred.erase(preferred.begin());
		}
		else {
			// Try next platform
			platforms.erase(platforms.begin());
		}
	}
	release_ensure_msg(0, "No OpenCL-device found");
}



util::DynArray<ClState::Platform> ClState::queryPlatforms(){

	char8 chBuffer[1024];
	cl_uint platform_count= 0;
	cl_int ciErrNum;
	cl_uint i = 0;
	
	
	util::DynArray<Platform> platforms;

	// Get OpenCL platform count
	ciErrNum = clGetPlatformIDs (0, NULL, &platform_count);
	if (ciErrNum != CL_SUCCESS)
	{

		throw(global::Exception("clGetPlatformID::(..): Error in clGetPlatformIDs(..) call: %i, %s", ciErrNum, gClState->getErrorString(ciErrNum)));

	}
	else
	{
		if(platform_count == 0)
		{

			throw(global::Exception("clGetPlatformID::(..): No OpenCL platforms found"));
		}
		else
		{
			std::vector<cl_platform_id> clPlatformIDs(platform_count);

			// get platform info for each platform and trap the NVIDIA platform if found
			ciErrNum = clGetPlatformIDs (platform_count, clPlatformIDs.data(), NULL);

			
			for (i=0; i<platform_count; i++){
				ciErrNum = clGetPlatformInfo (clPlatformIDs[i], CL_PLATFORM_NAME, 1024, &chBuffer, NULL);
				
				Platform p;
				
				p.id= clPlatformIDs[i];
				p.name= chBuffer;
			
				platforms.pushBack(p);

			}

		}
	}



	return platforms;
}

util::DynArray<ClState::Device> ClState::queryPlatformDevices(const Platform& p){
	// Querytetään laitteet
	
	util::DynArray<Device> devices;
	
	cl_int err2=0;
	
	cl_uint device_count;
	
	err2 = clGetDeviceIDs(p.id, CL_DEVICE_TYPE_ALL, 0, NULL, &device_count);
	errorCheck("ClState::ClState(): Error on clGetDeviceIDs 1: ", err2);

	std::vector<cl_device_id> device_ids(device_count);

	err2 = clGetDeviceIDs(p.id, CL_DEVICE_TYPE_ALL, device_count, device_ids.data(), NULL);
	errorCheck("CLstate::ClState(): Error on clGetDeviceIDs 2: ", err2);
	
	
	for (uint32 i=0; i<device_count; ++i){
		Device d;
		d.id= device_ids[i];
		d.platform= p;
		
		
		
		size_t ret_size;
		
		cl_bool available;
		
		
		// Onko käytettävissä
		err2= clGetDeviceInfo(d.id, CL_DEVICE_AVAILABLE, sizeof(available), &available, &ret_size);
		errorCheck("ClState::queryPlatformDevices(): Error on clGetDeviceInfo: ", err2);
		
		if (!available) continue;
		
		
		
		// Onko kääntötuki
		err2= clGetDeviceInfo(d.id, CL_DEVICE_COMPILER_AVAILABLE, sizeof(available), &available, &ret_size);
		errorCheck("ClState::queryPlatformDevices(): Error on clGetDeviceInfo: ", err2);
	
		if (!available) continue;
		
		
		// Minkä tyyppinen laite
		
		cl_device_type type[5];
		err2= clGetDeviceInfo(d.id, CL_DEVICE_TYPE, sizeof(type), &type, &ret_size);
		errorCheck("ClState::queryPlatformDevices(): Error on clGetDeviceInfo: ", err2);
		
		d.type= Device::Other;
		d.isDefault= false;
		//print(debug::Ch::OpenCL, debug::Vb::Trivial, "CL_DEVICE_TYPE ret: %i", ret_size);
		for (uint32 j=0; j<ret_size/sizeof(cl_device_type); ++j){
			//print(debug::Ch::OpenCL, debug::Vb::Trivial, "   %i", type[j]);
			switch (type[j]){
				case CL_DEVICE_TYPE_DEFAULT: d.isDefault= true; break;
				case CL_DEVICE_TYPE_CPU: d.type= Device::Cpu; break;
				case CL_DEVICE_TYPE_GPU: d.type= Device::Gpu; break;
				case CL_DEVICE_TYPE_ACCELERATOR: d.type= Device::Accelerator; break;
				
				default: break;
			}

		}
		
		
		// Laitteen nimi
		
		char buf[512];
		
		err2= clGetDeviceInfo(d.id, CL_DEVICE_NAME, sizeof(buf), &buf, &ret_size);
		errorCheck("ClState::queryPlatformDevices(): Error on clGetDeviceInfo: ", err2);
		
		d.name= buf;
		
		
		devices.pushBack(d);
		
	}
	
	return devices;
}

ClState::Context ClState::createContext(const Device& d){

	Context c;
	c.device= d;
	
	cl_int err2=0;
	
	#if OS == OS_LINUX

		cl_context_properties properties[]=
		{
			CL_GL_CONTEXT_KHR,(cl_context_properties)glXGetCurrentContext(),
			CL_GLX_DISPLAY_KHR,(cl_context_properties)glXGetCurrentDisplay(),
			0
		};

		c.id = clCreateContext(properties, 1, &d.id, 0, 0, &err2);

	#elif OS == OS_WINDOWS

		cl_context_properties properties[] =
		{
			CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
			CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
			CL_CONTEXT_PLATFORM, (cl_context_properties)(platform.id),
			0
		};


		c.id = clCreateContext(properties, 1, &d.id, 0, 0, &err2);
	#elif OS == OS_MACOSX

		CGLContextObj kCGLContext = CGLGetCurrentContext();
		CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
		cl_context_properties props[] =
		{
			CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup,
			0
		};

		c.id = clCreateContext(props, 0, 0, NULL, NULL, &err2);

	#endif

	errorCheck("ClState::createContext(..): Error on clCreateContext(..):", err2);

	return c;
}


ClState::Program ClState::createProgram(const Context& c, const util::Str8& path){
	Program p;
	p.path= path;

	size_t ret_size=0;

	util::Str8 source= global::File::readText(path);
	const char8 *src[2]= { source.cStr(), 0 };
	
	cl_int err=0;
	p.id = clCreateProgramWithSource(c.id, 1, src, 0, &err);
	errorCheck("ClState::createProgram(..): clCreateProgramWithSource failed: ", err);

	err= clBuildProgram(p.id, 1, &c.device.id, "-cl-std=CL1.1", 0, 0);

	cl_build_status build_status;
	cl_int err2= clGetProgramBuildInfo(p.id, c.device.id, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL);
	errorCheck("ClState::createProgram(..): clGetProgramBuildInfo 1 failed: ", err2);
	
	if (build_status != CL_BUILD_SUCCESS){
		// Build failed, display log
		err2= clGetProgramBuildInfo(p.id, c.device.id, CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_size);
		errorCheck("ClState::compile(..): clGetProgramBuildInfo 2 failed: ", err2);

		ensure(ret_size > 0);
		util::DynArray<uint8> log;
		log.resize(ret_size);
		err2= clGetProgramBuildInfo(p.id, c.device.id, CL_PROGRAM_BUILD_LOG, log.size(), log.data(), NULL);
		errorCheck("ClState::compile(..): clGetProgramBuildInfo 3 failed: ", err2);

		print(debug::Ch::OpenCL, debug::Vb::Critical, "OpenCL program %s build log (%lu):\n%s", path.cStr(), (unsigned long)ret_size, log.data());
	}

	errorCheck("ClState::compile(..): clBuildProgram failed: ", err);
	
	return p;
}

ClState::Kernel ClState::createKernel(const Program& p, const util::Str8& kernelname){
	cl_int err=0;
	
	Kernel k;
	k.name= kernelname;
	k.id= clCreateKernel(p.id, kernelname.cStr(), &err);
	k.program= p;
	errorCheck("ClState::createKernel(..): clCreateKernel failed: ", err);
	
	return k;
}


void ClState::destroyProgram(Program& p){
	ensure(p.id);
	
	cl_int err= clReleaseProgram(p.id);
	errorCheck("ClState::destroyProgram(..): clReleaseProgram failed: ", err);

	p.id= 0;
}

void ClState::destroyKernel(Kernel& k){
	ensure(k.id);
	cl_int err= clReleaseKernel(k.id);
	errorCheck("ClState::destroyKernel(..): clReleaseKernel failed: ", err);
	k.id=0;
}


ClState::Buffer ClState::createFromGLBuffer(const Context& c, BufferFlag flag, uint32 gl_id){
	ensure(gl_id != 0);
	cl_int err= 0;
	Buffer b;
	b.id= clCreateFromGLBuffer(	c.id,
								flag,
								gl_id,
								&err);

	errorCheck("ClState::createFromGLBuffer(..): clCreateFromGLBuffer failed: ", err);
	return (b);
}

ClState::Buffer ClState::createFromGLTexture(
		const Context& c,
		BufferFlag flag,
		uint32 gl_target,
		uint32 gl_id){
	ensure(gl_id != 0);
	cl_int err= 0;
	Buffer b;
	// clCreateFromGLTexture*D is deprecated, but OpenCL 1.1 is what we use
	if (gl_target == GL_TEXTURE_2D){
		b.id= clCreateFromGLTexture2D(	c.id,
										flag,
										gl_target,
										0, // miplevel
										gl_id,
										&err);
	}
	else if (gl_target == GL_TEXTURE_3D){
		b.id= clCreateFromGLTexture3D(	c.id,
										flag,
										gl_target,
										0, // miplevel
										gl_id,
										&err);
	}
	else {
		// OpenCL 1.2 has clCreateFromGLTexture which would allow tex arrays
		throw global::Exception(
				"ClState::createFromGLTexture(..): unsupported tex target");
	}
	
	errorCheck("ClState::createFromGLTexture(..): clCreateFromGLTexture failed: ", err);
	return b;
}

void ClState::destroyBuffer(Buffer& b){
	ensure(b.id);
	cl_int err= clReleaseMemObject(b.id);
	errorCheck("ClState::destroyBuffer(..): clReleaseMemObject failed: ", err);
	b.id= 0;
}
	
	
	
void ClState::acquireBuffer(const CommandQueue& q, Buffer& buffer){
	cl_int err=0;
	err= clEnqueueAcquireGLObjects(q.id, 1, &buffer.id, 0, 0, 0);
	errorCheck("ClState::acquireBuffers(..): clEngueueAcquireGLObjects failed: ", err);
}


void ClState::releaseBuffer(const CommandQueue& q, Buffer& buffer){
	cl_int err=0;
	err= clEnqueueReleaseGLObjects(q.id, 1, &buffer.id, 0, 0, 0);
	errorCheck("ClState::releaseBuffers(..): clEngueueReleaseGLObjects failed: ", err);
}

ClState::CommandQueue ClState::createCommandQueue(const Device& d){
	cl_int err=0;
	
	CommandQueue c;
	c.context= context;
	c.id= clCreateCommandQueue(c.context.id, d.id, 0, &err);
	errorCheck("ClState::createCommandQueue(): Error on clCreateCommandQueue(..):", err);
	
	return c;
}

const char* ClState::getErrorString(cl_int error)
{

	switch (error) {
		case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
		case CL_SUCCESS:							return "Success!";
		case -1001:									return "Platform not found";
		case CL_DEVICE_NOT_FOUND:				  return "Device not found";
		case CL_DEVICE_NOT_AVAILABLE:			  return "Device not available";
		case CL_COMPILER_NOT_AVAILABLE:			  return "Compiler not available";
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:	  return "Memory object allocation failure";
		case CL_OUT_OF_RESOURCES:				  return "Out of resources";
		case CL_OUT_OF_HOST_MEMORY:				  return "Out of host memory";
		case CL_PROFILING_INFO_NOT_AVAILABLE:	  return "Profiling information not available";
		case CL_MEM_COPY_OVERLAP:				  return "Memory copy overlap";
		case CL_IMAGE_FORMAT_MISMATCH:			  return "Image format mismatch";
		case CL_IMAGE_FORMAT_NOT_SUPPORTED:		  return "Image format not supported";
		case CL_BUILD_PROGRAM_FAILURE:			  return "Program build failure";
		case CL_MAP_FAILURE:					  return "util::Map failure";
		case CL_INVALID_VALUE:					  return "Invalid value";
		case CL_INVALID_DEVICE_TYPE:			  return "Invalid device type";
		case CL_INVALID_PLATFORM:				  return "Invalid platform";
		case CL_INVALID_DEVICE:					  return "Invalid device";
		case CL_INVALID_CONTEXT:				  return "Invalid context";
		case CL_INVALID_QUEUE_PROPERTIES:		  return "Invalid queue properties";
		case CL_INVALID_COMMAND_QUEUE:			  return "Invalid command queue";
		case CL_INVALID_HOST_PTR:				  return "Invalid host pointer";
		case CL_INVALID_MEM_OBJECT:				  return "Invalid memory object";
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:  return "Invalid image format descriptor";
		case CL_INVALID_IMAGE_SIZE:				  return "Invalid image size";
		case CL_INVALID_SAMPLER:				  return "Invalid sampler";
		case CL_INVALID_BINARY:					  return "Invalid binary";
		case CL_INVALID_BUILD_OPTIONS:			  return "Invalid build options";
		case CL_INVALID_PROGRAM:				  return "Invalid program";
		case CL_INVALID_PROGRAM_EXECUTABLE:		  return "Invalid program executable";
		case CL_INVALID_KERNEL_NAME:			  return "Invalid kernel name";
		case CL_INVALID_KERNEL_DEFINITION:		  return "Invalid kernel definition";
		case CL_INVALID_KERNEL:					  return "Invalid kernel";
		case CL_INVALID_ARG_INDEX:				  return "Invalid argument index";
		case CL_INVALID_ARG_VALUE:				  return "Invalid argument value";
		case CL_INVALID_ARG_SIZE:				  return "Invalid argument size";
		case CL_INVALID_KERNEL_ARGS:			  return "Invalid kernel arguments";
		case CL_INVALID_WORK_DIMENSION:			  return "Invalid work dimension";
		case CL_INVALID_WORK_GROUP_SIZE:		  return "Invalid work group size";
		case CL_INVALID_WORK_ITEM_SIZE:			  return "Invalid work item size";
		case CL_INVALID_GLOBAL_OFFSET:			  return "Invalid global offset";
		case CL_INVALID_EVENT_WAIT_LIST:		  return "Invalid event wait list";
		case CL_INVALID_EVENT:					  return "Invalid event";
		case CL_INVALID_OPERATION:				  return "Invalid operation";
		case CL_INVALID_GL_OBJECT:				  return "Invalid OpenGL object";
		case CL_INVALID_BUFFER_SIZE:			  return "Invalid buffer size";
		case CL_INVALID_MIP_LEVEL:				  return "Invalid mip-map level";
		default:								  return "Unknown";
	}


}

void ClState::destroyCommandQueue(CommandQueue& q){
	ensure(q.id);
	cl_int err= clReleaseCommandQueue(q.id);
	errorCheck("clReleaseCommandQueue", err);
	q.id= 0;
}

void ClState::enqueueWork(const Work& w, const CommandQueue& q){
	ensure(q.id != 0);
	size_t g_w_s= w.globalSize;
	size_t l_w_s= w.localSize;
	cl_int err= clEnqueueNDRangeKernel(q.id, w.kernel.id, (cl_uint)1, NULL, &g_w_s, &l_w_s, 0, NULL, NULL);
	errorCheck("clEnqueueNDRangeKernel", err);
}

void ClState::enqueueReadBuffer(const CommandQueue& q,
								Buffer& buffer,
								bool blocking,
								SizeType offset,
								SizeType size,
								void* host_ptr){
	cl_int err= 
		clEnqueueReadBuffer(	q.id,
								buffer.id,
								blocking,
								offset,
								size,
								host_ptr,
								0,
								nullptr,
								nullptr);
	errorCheck("clEnqueueRead", err);
}

void ClState::flush(const CommandQueue& q){
	ensure(q.id != 0);
	cl_int err= clFlush(q.id);
	errorCheck("clFlush", err);
}

void ClState::finish(const CommandQueue& q){
	ensure(q.id != 0);
	cl_int err= clFinish(q.id);
	errorCheck("clFinish", err);
}


void ClState::errorCheck(util::Str8 str, int32 err){
	if (err != CL_SUCCESS){
		util::Str8 msg= str + ": ";
		msg +=	getErrorString(err);
		release_ensure_msg(0, "%s", msg.cStr());
	}
}

} // hardware
} // clover