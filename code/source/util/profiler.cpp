#include "profiler.hpp"
#include "util/ensure.hpp"
#include "util/thread.hpp" // setThreadPriority
#include "util/time.hpp" // util::asMicroseconds

#include <chrono>
#include <type_traits>
#include <iostream>
#include <sstream>

namespace clover {
namespace util {

using namespace detail;
using ThreadUid= Profiler::ThreadUid;

// In gcc 4.8 thread_local creates a wrapper when
// It is not function-local, and,
// - it is extern, or
// - the type has a non-trivial destructor (which is not allowed for __thread variables), or
// - the type variable is initialized by a non-constant-expression
//	(which is also not allowed for __thread variables)

// So only POD thread_locals are cool.

/// Contains all thread local stuff needed for profiling
struct Profiler::ThreadLocalInfo {
	static constexpr SizeType maxDepth= 128;

	/// Callinfo
	/// @warning Assuming that reading and writing pointers are atomic operations!
	BlockInfo* frames[maxDepth];
	// Access only from info-modifying-thread
	SizeType freeFrame;

	// ThreadUid doesn't have duplicates as std::thread::id could have
	/// @warning Assuming that reading and writing ThreadUids are atomic operations!
	ThreadUid threadUid;

	/// Used for joining stacks of synchronous threads
	/// This thread sets
	ThreadUid superThreadUid;

	/// Used for joining stacks of synchronous threads
	/// Sub thread sets
	ThreadUid subThreadUid;
};

/// Keeps profiler up to date of thread call infos lifetimes
class Profiler::ThreadRaii {
public:
	ThreadRaii();
	~ThreadRaii();
};

/// @note Initalization order might matter

using RMutex= std::recursive_mutex;
using RLockGuard= std::lock_guard<RMutex>;
constexpr ThreadUid threadUidNone= 0;

struct ThreadIdentity {
	std::thread::id id;
	ThreadUid uid; /// id's aren't unique
	Profiler::ThreadLocalInfo* info;
};

static std::array<ThreadIdentity, 512> g_threads;
static ThreadUid g_freeThreadUid= threadUidNone + 1;
static RMutex g_threadInfosMutex;

static thread_local Profiler::ThreadLocalInfo threadLocalInfo;
static thread_local Profiler::ThreadRaii threadRaii;

static_assert(	std::is_pod<Profiler::ThreadLocalInfo>::value,
		"Must be POD to have minimal perf cost with thread_local");

Profiler::ThreadRaii::ThreadRaii(){ Profiler::onThreadBegin(threadLocalInfo); }
Profiler::ThreadRaii::~ThreadRaii(){ Profiler::onThreadEnd(threadLocalInfo); }

static ThreadUid getThreadUid(std::thread::id id){
	RLockGuard g(g_threadInfosMutex);
	for (SizeType i= 0; i < g_threads.size(); ++i) {
		if (g_threads[i].id == id)
			return g_threads[i].uid;
	}
	ensure_msg(false, "Thread UID not found");
	std::abort();
}

static Profiler::ThreadLocalInfo& getThreadLocalInfo(ThreadUid uid){
	RLockGuard g(g_threadInfosMutex);
	for (SizeType i= 0; i < g_threads.size(); ++i) {
		if (g_threads[i].uid == uid)
			return *NONULL(g_threads[i].info);
	}
	ensure_msg(false, "Thread LocalInfo not found");
	std::abort();
}

static void staticThreadLocalInfoInit(){
	// Thread_local objects are constructed when first used (gcc 4.8.1)
	// Not sure if this is correct behaviour
	threadRaii;
}

//
// static Profiler
//

void Profiler::onThreadBegin(ThreadLocalInfo& info){
	if (PROFILING_ENABLED){
		RLockGuard g(g_threadInfosMutex);

		info.threadUid= g_freeThreadUid;
		++g_freeThreadUid;

		// Overflow of uids - not so unique anymore
		if (g_freeThreadUid == threadUidNone)
			++g_freeThreadUid;

		// Add thread to global list
		bool added= false;
		for (SizeType i= 0; i < g_threads.size(); ++i) {
			auto& t= g_threads[i];
			if (t.uid == threadUidNone) {
				t.id= std::this_thread::get_id();
				t.uid= info.threadUid;
				t.info= &info;
				added= true;
				break;
			}
		}
		ensure_msg(added, "Too many threads");
	}
}

void Profiler::onThreadEnd(ThreadLocalInfo& info){
	if (PROFILING_ENABLED){
		RLockGuard g(g_threadInfosMutex);

		ensure(threadLocalInfo.subThreadUid == threadUidNone);

		bool removed= false;
		for (SizeType i= 0; i < g_threads.size(); ++i) {
			auto& t= g_threads[i];
			if (t.uid == info.threadUid) {
				t.uid= threadUidNone;
				t.info= nullptr;
				removed= true;
				break;
			}
		}
		ensure(removed);
	}
}

void Profiler::onBlockEnter(BlockInfo& block){
	if (PROFILING_ENABLED){
		staticThreadLocalInfoInit();

		if (threadLocalInfo.freeFrame < ThreadLocalInfo::maxDepth){
			threadLocalInfo.frames[threadLocalInfo.freeFrame]= &block;
		} else {
			release_ensure_msg(0, "Profiler::onBlockEnter(..): out of info");
		}
		++threadLocalInfo.freeFrame;

		// Make sure that info is seen as coherent in other thread
		std::atomic_thread_fence(std::memory_order_release);
	}
}

void Profiler::onBlockExit(){
	if (PROFILING_ENABLED){
		--threadLocalInfo.freeFrame;
		if (threadLocalInfo.freeFrame < ThreadLocalInfo::maxDepth)
			threadLocalInfo.frames[threadLocalInfo.freeFrame]= nullptr;

		// Make sure that info is seen as coherent in other thread
		std::atomic_thread_fence(std::memory_order_release);
	}
}

void Profiler::setSuperThread(std::thread::id super){
	staticThreadLocalInfoInit();

	threadLocalInfo.superThreadUid= getThreadUid(super);
}

void Profiler::onStackJoin(){

	ensure(threadLocalInfo.superThreadUid != threadUidNone);
	ensure_msg(threadLocalInfo.threadUid != threadUidNone, "Thread not initialized");

	ThreadLocalInfo& super_info= getThreadLocalInfo(threadLocalInfo.superThreadUid);
	ensure_msg(
			super_info.subThreadUid == threadUidNone,
			"Multiple simultaneous synchronous sub threads");
	super_info.subThreadUid= threadLocalInfo.threadUid;
}

void Profiler::onStackDetach(){
	ensure(threadLocalInfo.superThreadUid != threadUidNone);
	ensure_msg(threadLocalInfo.threadUid != threadUidNone, "Thread not initialized");

	ThreadLocalInfo& super_info= getThreadLocalInfo(threadLocalInfo.superThreadUid);
	ensure_msg(
			super_info.subThreadUid == threadLocalInfo.threadUid,
			"Multiple simultaneous synchronous sub threads");
	super_info.subThreadUid= threadUidNone;
}

void Profiler::onSystemMemAlloc(){
	if (PROFILING_ENABLED){
		staticThreadLocalInfoInit();

		for (SizeType i= 0; i < threadLocalInfo.freeFrame; ++i) {
			ensure(i < ThreadLocalInfo::maxDepth);
			BlockInfo& b= *NONULL(threadLocalInfo.frames[i]);
			++b.inclusiveMemAllocs;
			if (i + 1 == threadLocalInfo.freeFrame)
				++b.exclusiveMemAllocs;
		}

		// Make sure that info is seen as coherent in other thread
		std::atomic_thread_fence(std::memory_order_release);
	}
}

Profiler::StackFrames Profiler::readStack(uint64& e_mallocs, uint64& i_mallocs, const ThreadLocalInfo& info){
	e_mallocs= 0;
	i_mallocs= 0;
	ensure(info.threadUid != threadUidNone);

	Profiler::StackFrames frames;
	frames.reserve(ThreadLocalInfo::maxDepth);

	for (SizeType i= 0; i < ThreadLocalInfo::maxDepth; ++i) {
		std::atomic_thread_fence(std::memory_order_acquire);

		// Don't care if frames change during the loop, because
		// longest taking operations will be captured correctly
		// at average, and that's the whole point of sampling
		frames.push_back(info.frames[i]);

		if (frames.back() == nullptr) {
			frames.pop_back();
			break;
		} else {
			e_mallocs= frames.back()->exclusiveMemAllocs;
			i_mallocs= frames.back()->inclusiveMemAllocs;
		}
	}

	// info.subThreadUid can change during if to threadUidNone
	ThreadUid sub_uid= info.subThreadUid;
	if (sub_uid != threadUidNone){
		// Append frames from sub thread
		uint64 sub_e_mallocs= 0;
		uint64 sub_i_mallocs= 0;
		Profiler::StackFrames sub_frames=
			readStack(
					sub_e_mallocs, sub_i_mallocs,
					getThreadLocalInfo(sub_uid));
		frames.insert(frames.end(), sub_frames.begin(), sub_frames.end());

		i_mallocs += sub_i_mallocs;
		e_mallocs += sub_e_mallocs;
	}

	return frames;
}

//
// Profiler::Result
//

Profiler::Result::Result(const Profile& profile, real64 min_share)
	: measureCount(maxSamplesPerThread(profile)) // Assuming that main thread lives during profiling
	, threadIdsToUids(profile.threadIdsToUids){

	for (auto& p1 : profile.samplesByThread){
		ThreadUid thread= p1.first;
		const Samples& samples= p1.second;
		SizeType total= totalSampleCount(samples);
		for (auto& p2 : samples){
			const StackFrames& stack= p2.first;
			const Sample& s= p2.second;

			real64 share= (double)s.count/total;
			if (share >= min_share) {
				Label label= lastLabel(stack);

				StackResult result;
				result.share= share;
				result.iMemAllocs= s.totalIMemAllocs - s.iMemAllocsAtStartup;
				result.eMemAllocs= s.totalEMemAllocs - s.eMemAllocsAtStartup;
				stackResults[StackInfo{thread, stack}]= result;

				if (!label.empty())
					labelResults[LabelInfo{thread, std::move(label)}] += share;
			}
		}
	}
}

auto Profiler::Result::getSortedSamples(std::thread::id thread_id) const -> std::vector<Item> {
	std::vector<Item> samples;
	for (auto& p : flippedMap<std::greater<StackResult>>(stackResults)){
		const StackInfo& info= p.second;
		const StackResult& result= p.first;
		if (info.threadUid != threadIdsToUids.at(thread_id))
			continue;

		samples.push_back(
				Item{	sampleStr(info),
						result.share,
						result.iMemAllocs,
						result.eMemAllocs});
	}
	return samples;
}

auto Profiler::Result::getSortedLabels(std::thread::id thread_id) const -> std::vector<Item> {
	std::vector<Item> labels;
	for (auto& p : flippedMap<std::greater<real64>>(labelResults)){
		const LabelInfo& info= p.second;
		if (info.threadUid != threadIdsToUids.at(thread_id))
			continue;

		labels.push_back(Item{info.label, p.first});
	}
	return labels;
}

SizeType Profiler::Result::totalSampleCount(const Profile& profile){
	SizeType total= 0;
	for (const auto& p1 : profile.samplesByThread){
		const Samples& samples= p1.second;
		total += totalSampleCount(samples);
	}
	return total;
}

SizeType Profiler::Result::totalSampleCount(const Samples& samples){
	SizeType total= 0;
	for (auto& p : samples){
		total += p.second.count; // += sample count
	}
	return total;
}

SizeType Profiler::Result::maxSamplesPerThread(const Profile& profile){
	SizeType largest_sample_count= 0;
	for (auto& p1 : profile.samplesByThread){
		const Samples& samples= p1.second;
		SizeType thread_sample_count= totalSampleCount(samples);
		if (thread_sample_count > largest_sample_count)
			largest_sample_count= thread_sample_count;
	}
	return largest_sample_count;
}

std::string Profiler::Result::funcStr(const char* funcname){
	SizeType start= 0;
	while (funcname[start] != 0 && funcname[start] != ' ')
		++start;
	while (funcname[start] != 0 && funcname[start] == ' ')
		++start;
	return funcname + start;
}

std::string Profiler::Result::sampleStr(const StackInfo& s){
	if (s.stackFrames.empty())
		return "unknown";

	bool first= true;
	std::string last_func;
	std::stringstream ss;
	for (auto& block : s.stackFrames){
		if (!first)
			ss << "-> ";

		std::string func= funcStr(block->funcName);
		if (func != last_func)
			ss << funcStr(block->funcName);
		last_func= func;

		ss << "[" << block->line;
		if (block->label){
			ss << "," << block->label;
		}
		ss << "]";

		first= false;
	}
	return ss.str();
}

auto Profiler::Result::lastLabel(const StackFrames& s) -> Label {
	const char* block_label= nullptr;
	for (auto& block : s){
		ensure(block);
		if (block->label)
			block_label= block->label;
	}
	return block_label ? block_label : "";
}

//
// Profiler
//

Profiler::Profiler(real64 samplerate)
	: keepProfiling(true)
	, profileInterval(1.0/samplerate)
	, profilingThread(std::bind(&Profiler::profileLoop, this)){

	if (PROFILING_ENABLED){
		setThreadPriority(profilingThread, ThreadPriority::Critical);
	}

	if (!gProfiler)
		gProfiler= this;
}

Profiler::~Profiler(){
	keepProfiling= false;
	profilingThread.join();

	if (gProfiler == this)
		gProfiler= nullptr;
}

Profiler::Result Profiler::popResult(real64 min_share){
	RLockGuard g(profileMutex);
	auto result= Result(profile, min_share);
	profile= Profile{};
	return result;
}

void Profiler::profileLoop(){
	if (!PROFILING_ENABLED)
		return;

	while (keepProfiling){
		{ LockGuard g1(g_threadInfosMutex); LockGuard g2(profileMutex);
			for (SizeType i= 0; i < g_threads.size(); ++i) {
				auto& t= g_threads[i];
				if (t.uid == threadUidNone)
					continue;

				// Just overwrite if std::thread::id is reused
				profile.threadIdsToUids[t.id]= t.uid;

				ThreadLocalInfo& info= *NONULL(t.info);
				uint64 e_malloc_count= 0;
				uint64 i_malloc_count= 0;
				Sample& s=
					profile.samplesByThread[info.threadUid][
						readStack(e_malloc_count, i_malloc_count, info)];
				++s.count;

				s.totalIMemAllocs= i_malloc_count;
				if (s.iMemAllocsAtStartup == 0)
					s.iMemAllocsAtStartup= s.totalIMemAllocs;

				s.totalEMemAllocs= e_malloc_count;
				if (s.eMemAllocsAtStartup == 0)
					s.eMemAllocsAtStartup= s.totalEMemAllocs;
			}

		}
		std::this_thread::sleep_for(util::asMicroseconds(profileInterval));
	}
}

Profiler* gProfiler= nullptr;

} // util
} // clover
