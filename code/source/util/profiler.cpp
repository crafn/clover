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
// 	(which is also not allowed for __thread variables)

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

using Mutex= std::recursive_mutex;
using LockGuard= std::lock_guard<Mutex>;
constexpr ThreadUid threadUidNone= 0;

/// Maps thread::ids of currently running threads to all-time unique ids
static std::map<std::thread::id, ThreadUid> g_threadIdsToUids;
static std::map<ThreadUid, Profiler::ThreadLocalInfo*> g_threadLocalInfos;
static ThreadUid g_freeThreadUid= threadUidNone + 1;
static Mutex g_threadInfosMutex;

static thread_local Profiler::ThreadLocalInfo threadLocalInfo;
static thread_local Profiler::ThreadRaii threadRaii;

static_assert(	std::is_pod<Profiler::ThreadLocalInfo>::value,
		"Must be POD to have minimal perf cost with thread_local");

Profiler::ThreadRaii::ThreadRaii(){ Profiler::onThreadBegin(threadLocalInfo); }
Profiler::ThreadRaii::~ThreadRaii(){ Profiler::onThreadEnd(threadLocalInfo); }

static ThreadUid getThreadUid(std::thread::id id){
	LockGuard g(g_threadInfosMutex);
	ensure(g_threadIdsToUids.find(id) != g_threadIdsToUids.end());
	return g_threadIdsToUids.at(id);
}

static Profiler::ThreadLocalInfo& getThreadLocalInfo(ThreadUid uid){
	LockGuard g(g_threadInfosMutex);
	ensure(g_threadLocalInfos.find(uid) != g_threadLocalInfos.end());
	return *g_threadLocalInfos.at(uid);
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
		LockGuard g(g_threadInfosMutex);

		info.threadUid= g_freeThreadUid;
		++g_freeThreadUid;

		// Overflow of uids - not so unique anymore
		if (g_freeThreadUid == threadUidNone)
			++g_freeThreadUid;

		/// @todo Remove memory allocations so profiling can be used in `new`
		g_threadIdsToUids[std::this_thread::get_id()]= info.threadUid;
		g_threadLocalInfos[info.threadUid]= &info;
	}
}

void Profiler::onThreadEnd(ThreadLocalInfo& info){
	if (PROFILING_ENABLED){
		LockGuard g(g_threadInfosMutex);

		ensure(threadLocalInfo.subThreadUid == threadUidNone);
		ensure(g_threadLocalInfos.find(info.threadUid) != g_threadLocalInfos.end());
		ensure(g_threadIdsToUids.find(std::this_thread::get_id()) != g_threadIdsToUids.end());
		ensure(	threadLocalInfo.superThreadUid == threadUidNone ||
					g_threadLocalInfos[threadLocalInfo.superThreadUid]->subThreadUid
					!= threadLocalInfo.threadUid);

		g_threadLocalInfos.erase(g_threadLocalInfos.find(info.threadUid));
		g_threadIdsToUids.erase(g_threadIdsToUids.find(std::this_thread::get_id()));
	}
}

void Profiler::onBlockEnter(BlockInfo& block){
	if (PROFILING_ENABLED){
			staticThreadLocalInfoInit();

			if (threadLocalInfo.freeFrame < ThreadLocalInfo::maxDepth){
			threadLocalInfo.frames[threadLocalInfo.freeFrame]= &block;
		}
		else {
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


Profiler::Sample Profiler::readStack(const ThreadLocalInfo& info){
	ensure(info.threadUid != threadUidNone);

	Profiler::Sample frames;
	frames.reserve(ThreadLocalInfo::maxDepth);

	for (SizeType i= 0; i < ThreadLocalInfo::maxDepth; ++i){
		std::atomic_thread_fence(std::memory_order_acquire);

		// Don't care if frames change during the loop, because
		// longest taking operations will be captured correctly
		// at average, and that's the whole point of sampling
		frames.push_back(info.frames[i]);

		if (frames.back() == nullptr){
			frames.pop_back();
			break;
		}
	}

	// info.subThreadUid can change during if to threadUidNone
	ThreadUid sub_uid= info.subThreadUid;
	if (sub_uid != threadUidNone){
		// Append frames from sub thread
		Profiler::Sample sub_frames= readStack(getThreadLocalInfo(sub_uid));
		frames.insert(frames.end(), sub_frames.begin(), sub_frames.end());
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
			const Sample& sample= p2.first;
			SizeType count= p2.second;

			real64 share= (double)count/total;
			if (share >= min_share)
				addSample(sample, thread, share);
		}
	}
}

auto Profiler::Result::getSortedSamples(std::thread::id thread_id) const -> std::vector<Item> {
	std::vector<Item> samples;
	for (auto& p : flippedMap<std::greater<real64>>(sampleResults)){
		const SampleInfo& info= p.second;
		if (info.threadUid != threadIdsToUids.at(thread_id))
			continue;

		samples.push_back(Item{sampleStr(info), p.first});
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

void Profiler::Result::addSample(Sample sample, ThreadUid thread, real64 share){
	Label label= lastLabel(sample);

	sampleResults[SampleInfo{thread, std::move(sample)}]= share;

	if (!label.empty())
		labelResults[LabelInfo{thread, std::move(label)}] += share;
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
		total += p.second; // += sample count
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

std::string Profiler::Result::sampleStr(const SampleInfo& s){
	if (s.sample.empty())
		return "unknown";

	bool first= true;
	std::string last_func;
	std::stringstream ss;
	for (auto& block : s.sample){
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

auto Profiler::Result::lastLabel(const Sample& s) -> Label {
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
	LockGuard g(profileMutex);
	auto result= Result(profile, min_share);
	profile= Profile{};
	return result;
}

SizeType Profiler::getCallStackCount() const {
	LockGuard g(g_threadInfosMutex);
	return g_threadLocalInfos.size();
}

void Profiler::profileLoop(){
	if (!PROFILING_ENABLED)
		return;

	while (keepProfiling){
		{ LockGuard g1(g_threadInfosMutex); LockGuard g2(profileMutex);
			// Just overwrite if std::thread::id is reused
			profile.threadIdsToUids.insert(g_threadIdsToUids.begin(), g_threadIdsToUids.end());

			for (const auto& pair : g_threadLocalInfos){
				ThreadLocalInfo& info= *pair.second;
				ensure(info.threadUid != threadUidNone);
				++profile.samplesByThread[info.threadUid][readStack(info)];
			}

		}
		std::this_thread::sleep_for(util::asMicroseconds(profileInterval));
	}
}

Profiler* gProfiler= nullptr;

} // util
} // clover