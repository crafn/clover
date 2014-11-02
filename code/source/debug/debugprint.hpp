#ifndef CLOVER_DEBUGPRINT_HPP
#define CLOVER_DEBUGPRINT_HPP

#include "build.hpp"

#include <bitset>
#include <boost/thread/mutex.hpp>

#ifdef __GNUC__
#define PRINTF_FORMAT(stringIndex, firstToCheck) \
  __attribute__ ((format (printf, (stringIndex), (firstToCheck))))
#else
#define PRINTF_FORMAT(a, b)
#endif

namespace clover {
namespace debug {

/// Channel of message
enum class Ch {
	Anim,
    WE,
    OpenGL,
    OpenCL,
    Device,
    Phys,
    Audio,
    Event,
	Visual,
    Resources,
	Save,
	Gui,
	Ui,
	WorldChunk,
	Script,
	Net,
	Nodes,
	Dev,
    General,
    Last
};

/// Verbosity level
enum class Vb {
	Trivial,
	Moderate,
    Critical,
	Last
};

/// Handles all debug printing
class DebugPrint {
public:
	static constexpr SizeType channelCount= static_cast<SizeType>(Ch::Last);
	static constexpr SizeType verbosityCount= static_cast<SizeType>(Vb::Last);

	static util::Str8& getChannelString(Ch id);
	static util::Str8& getVerbosityString(Vb v);

    DebugPrint();

    void setVerbosity(Vb);
	Vb getVerbosity() const { return verbosity; }

    void setChannelActive(Ch, bool);
	bool isChannelActive(Ch) const;

    /// Appends a row to stdout and log
    void operator()(Ch c, Vb v, const char* str, ...);

	struct Message {
		Ch channel;
		Vb verbosity;
		
		std::string string;
	};

	typedef std::vector<Message> Buffer;
	
	void updateBuffer();

private:
	mutable boost::mutex mutex;

    std::bitset<static_cast<SizeType>(Ch::Last)> filter;
    Vb verbosity;
	
	Buffer buffer;
};

extern DebugPrint gDebugPrint;

/// Convenience function for debug printing
/// @example print(debug::Ch::Audio, debug::Vb::Trivial, "test %i", 5);
template <typename... Args>
void print(Ch ch, Vb vb, const char* fmt, Args&&... args){
	gDebugPrint(ch, vb, fmt, std::forward<Args>(args)...);
}

} // debug
} // clover

#endif // CLOVER_DEBUGPRINT_HPP