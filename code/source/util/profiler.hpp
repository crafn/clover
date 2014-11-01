#ifndef CLOVER_UTIL_PROFILER_HPP
#define CLOVER_UTIL_PROFILER_HPP

#include "build.hpp"
#include "profiling.hpp"
#include "util/map.hpp"

#include <atomic>
#include <future>
#include <mutex>
#include <set>
#include <string>
#include <vector>
#include <map>

namespace clover {
namespace util {

using namespace detail;

/// Manages sample-based profiling information
class Profiler {
public:
	class ThreadLocalInfo;
	class ThreadRaii;

	/// @warning ThreadUid should have atomic read/write
	using ThreadUid= uint32;
	using Label= std::string;
	using Sample= std::vector<BlockInfo*>;
	/// <sample, count>
	using Samples= std::map<Sample, SizeType>;
	struct Profile {
		std::map<ThreadUid, Samples> samplesByThread;
		std::map<std::thread::id, ThreadUid> threadIdsToUids;
	};

	struct Result {
	public:
		struct Item {
			std::string id;
			/// Approximate portion of time spent within this item during
			/// profiling. Shares don't overlap, so sum ~ 1.0 within a thread
			real64 share;
		};

		Result(const Profile& profile, real64 min_share);

		std::vector<Item> getSortedSamples(std::thread::id thread_id) const;
		std::vector<Item> getSortedLabels(std::thread::id thread_id) const;

		SizeType getMeasureCount() const { return measureCount; }

	private:
		struct SampleInfo {
			ThreadUid threadUid;
			Sample sample;
			bool operator<(const SampleInfo& other) const {
				return	std::make_pair(threadUid, sample) <
						std::make_pair(other.threadUid, other.sample);
			}
		};

		struct LabelInfo {
			ThreadUid threadUid;
			Label label;
			bool operator<(const LabelInfo& other) const {
				return	std::make_pair(threadUid, label) <
						std::make_pair(other.threadUid, other.label);
			}
		};

		void addSample(Sample sample, ThreadUid thread, real64 share);

		static SizeType totalSampleCount(const Profile& p);
		static SizeType totalSampleCount(const Samples& s);
		static SizeType maxSamplesPerThread(const Profile& p);
		static std::string funcStr(const char* funcname);
		static std::string sampleStr(const SampleInfo& s);
		static Label lastLabel(const Sample& s);

		SizeType measureCount;

		/// <sample, share>
		Map<SampleInfo, real64> sampleResults;

		/// Results for labels are joined from sample shares
		/// If a sample contains more than one label, share of the
		/// sample is accounted for the last (deepest) label
		/// <label, share>
		Map<LabelInfo, real64> labelResults;

		/// Copy from Profile
		Map<std::thread::id, ThreadUid> threadIdsToUids;
	};

	Profiler(real64 samplerate);
	~Profiler();

	Result popResult(real64 min_share);

	SizeType getCallStackCount() const;

protected: // Static, possibly called from other threads

	friend class ThreadLocalInfo;
	friend class ThreadRaii;
	friend class util::detail::BlockProfiler;
	friend class util::detail::StackJoiner;
	friend class util::detail::StackDetacher;
	friend void util::detail::setSuperThread(std::thread::id);

	static void onThreadBegin(ThreadLocalInfo& info);
	static void onThreadEnd(ThreadLocalInfo& info);

	static void onBlockEnter(BlockInfo& block);
	static void onBlockExit();

	static void setSuperThread(std::thread::id super);
	static void onStackJoin();
	static void onStackDetach();

private: // Static
	/// @warning Assuming that threads can't be destroyed during this function
	static Sample readStack(const ThreadLocalInfo& stack);

private: // Non-static
	using Mutex= std::recursive_mutex;
	using LockGuard= std::lock_guard<Mutex>;

	void profileLoop();

	std::atomic<bool> keepProfiling;
	std::atomic<real64> profileInterval;
	Mutex profileMutex;
	Profile profile;
	std::thread profilingThread;
};

extern Profiler* gProfiler;

} // util
} // clover

#endif // CLOVER_UTIL_PROFILER_HPP