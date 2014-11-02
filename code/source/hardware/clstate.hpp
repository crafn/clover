#ifndef CLOVER_HARDWARE_CLSTATE_HPP
#define CLOVER_HARDWARE_CLSTATE_HPP

#include "build.hpp"
#include "util/string.hpp"
#include "util/dyn_array.hpp"
#include "util/ensure.hpp"

#include <CL/cl.h>

namespace clover {
namespace hardware {

/// OpenCL context wrapper
class ClState {
public:
	struct Platform {
		cl_platform_id id;

		util::Str8 name;
	};

	struct Device {
		cl_device_id id;

		enum Type {
			Gpu,
			Cpu,
			Accelerator,
			Other
		};

		Platform platform;
		Type type;
		bool isDefault;
		util::Str8 name;
	};

	struct Context {
		Context() : id(nullptr), device() {}

		Context(const Context& ctx) : id(ctx.id), device(ctx.device) {
			if (id) clRetainContext(id);
		}

		Context & operator=(const Context& ctx) {
			if (id) clReleaseContext(id);
			id= ctx.id;
			device= ctx.device;
			if (id) clRetainContext(id);
			return *this;
		}

		Context(Context&& ctx) : id(ctx.id), device(ctx.device) {
			ctx.id = nullptr;
			ctx.device = Device();
		}

		Context & operator=(Context&& ctx) {
			if (id) clReleaseContext(id);
			id= ctx.id;
			device= ctx.device;
			ctx.id = nullptr;
			ctx.device = Device();
			return *this;
		}

		~Context() {
			if (id) clReleaseContext(id);
		}

		cl_context id;

		Device device;
	};

	struct CommandQueue {
		cl_command_queue id;

		Context context;
	};

	struct Program {
		util::Str8 path;
		cl_program id;
	};

	struct Kernel {
		util::Str8 name;
		cl_kernel id;
		Program program;
	};

	struct Work {
		Kernel kernel;
		int32 globalSize;
		int32 localSize;
	};

	enum BufferFlag {
		ReadWrite= CL_MEM_READ_WRITE,
		WriteOnly= CL_MEM_WRITE_ONLY,
		ReadOnly= CL_MEM_READ_ONLY,
		UseHostPtr= CL_MEM_USE_HOST_PTR,
		AllocHostPtr= CL_MEM_ALLOC_HOST_PTR,
		CopyHostPtr= CL_MEM_COPY_HOST_PTR
	};

	struct Buffer {
		cl_mem id;
	};

	ClState();

	void choosePlatformWiselyAndCreateContextNicely();

	const Context& getDefaultContext() const { return context; }
	const Platform& getDefaultPlatform() const { return platform; }
	const Device& getDefaultDevice() const { return device; }

	Program createProgram(const Context& d, const util::Str8& path);
	Kernel createKernel(const Program& p, const util::Str8& kernelname);

	template <typename T>
	Buffer createBuffer(const Context& c, BufferFlag flag, T& first, uint32 count=1);

	Buffer createFromGLBuffer(const Context& c, BufferFlag flag, uint32 gl_id);
	Buffer createFromGLTexture(	const Context& c,
								BufferFlag flag,
								uint32 gl_target,
								uint32 gl_id);

	void destroyProgram(Program& p);
	void destroyKernel(Kernel& k);
	void destroyBuffer(Buffer& b);

	void acquireBuffer(const CommandQueue& q, Buffer& buffer);
	void releaseBuffer(const CommandQueue& q, Buffer& buffer);

	CommandQueue createCommandQueue(const Device& d);
	void destroyCommandQueue(CommandQueue& q);

	void enqueueWork(const Work& w, const CommandQueue& q);

	void enqueueReadBuffer(	const CommandQueue& q,
							Buffer& buffer,
							bool blocking,
							SizeType offset,
							SizeType size,
							void* host_ptr);

	void flush(const CommandQueue& q);
	void finish(const CommandQueue& q);

	template <typename T>
	void setKernelArgument(const Kernel& kernel, uint32 arg_index, const T& value, uint32 count=1){
		cl_int err=0;
		size_t size= sizeof(value)*count;
		err= clSetKernelArg(kernel.id, arg_index, size, (const void*)&value);
		errorCheck("ClState::setKernelArgument(..): clSetKernelArg failed: ", err);
	}

	void errorCheck(util::Str8 str, int32 err);
	
private:
	const char* getErrorString(cl_int error);

	util::DynArray<Platform> queryPlatforms();
	util::DynArray<Device> queryPlatformDevices(const Platform& p);
	Context createContext(const Device& d);

	// Defaults
	Platform platform;
	Context context;
	Device device;
};


template <typename T>
ClState::Buffer ClState::createBuffer(const Context& c, BufferFlag flag, T& first, uint32 count){
	cl_int err=0;
	Buffer b;
	b.id= clCreateBuffer(	c.id,
							flag,
							count*sizeof(first),
							&first,
							&err);

	errorCheck("ClState::createBuffer(..): clCreateFromGLBuffer failed: ", err);
	return (b);
}

extern ClState* gClState;

} // hardware
} // clover

#endif // CLOVER_HARDWARE_CLSTATE_HPP