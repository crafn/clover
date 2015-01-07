#include "parallel_program.hpp"
#include "hardware/clstate.hpp"
#include "parallel_queue.hpp"
#include "debug/print.hpp"
#include "hardware/device.hpp"
#include "visual/vertexarrayobject.hpp"
#include "string.hpp"

namespace clover {
namespace util {

bool parallelProcessingSupported() { return hardware::gClState != nullptr; }

ParallelProgram::ParallelProgram(util::Str8 path):attachedQueue(0){
	program.id= 0;
	compile(path);
}

ParallelProgram::ParallelProgram():attachedQueue(0){
	program.id= 0;
}

ParallelProgram::ParallelProgram(ParallelProgram&& other){
	operator=(std::move(other));
}

ParallelProgram::~ParallelProgram(){
	ensure(hardware::gClState);
	
	for (auto& m : kernels)
		hardware::gClState->destroyKernel(m.kernel);
	kernels.clear();

	if (program.id != 0)
		hardware::gClState->destroyProgram(program);

	program.id= 0;
}

ParallelProgram& ParallelProgram::operator=(ParallelProgram&& other){
	program= std::move(other.program);
	kernels= std::move(other.kernels);
	attachedQueue= other.attachedQueue;

	other.kernels.clear();
	other.program.id= 0;
	other.attachedQueue= 0;

	return *this;
}

void ParallelProgram::compile(util::Str8 path){
	ensure(hardware::gClState);
	program= hardware::gClState->createProgram(hardware::gClState->getDefaultContext(), path);
}

util::ParallelKernel& ParallelProgram::createKernel(util::Str8 kernelname){
	ensure(hardware::gClState);
	ensure(program.id);

	kernels.pushBack(std::move(util::ParallelKernel(hardware::gClState->createKernel(program, kernelname))));
	if (attachedQueue)
		kernels.back().attachToQueue(*attachedQueue);

	return kernels.back();
}

void ParallelProgram::attachToQueue(util::ParallelQueue& q){
	ensure(program.id);
	
	attachedQueue= &q;

	if (!attachedQueue) return;
	for (auto& m : kernels){
		m.attachToQueue(*attachedQueue);
	}
}

} // util
} // clover