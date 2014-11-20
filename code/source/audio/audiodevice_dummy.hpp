#ifndef CLOVER_AUDIO_AUDIODEVICE_DUMMY_HPP
#define CLOVER_AUDIO_AUDIODEVICE_DUMMY_HPP

#include "audiodevice.hpp"
#include "build.hpp"
#include "outputstream.hpp"

/// @todo Replace with util::Thread
#include <thread>

namespace clover {
namespace audio {

/// Doesn't output anything, used when audio is disabled
class DummyAudioDevice : public AudioDevice {
public:
	DummyAudioDevice();
	virtual ~DummyAudioDevice();
	
	OutputStream& getOutputStream(){ return outputStream; }
	
private:
	/// Reads outputStream like normal audio device, but doesn't output any audio
	/// Runs in outputSimulatorThread
	void outputSimulator();
	
	OutputStream outputStream;
	std::atomic<bool> runSimulatorThread;
	std::thread outputSimulatorThread;
};

} // audio
} // clover

#endif // CLOVER_AUDIO_AUDIODEVICE_DUMMY_HPP
