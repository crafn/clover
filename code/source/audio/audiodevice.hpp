#ifndef CLOVER_AUDIO_AUDIODEVICE_HPP
#define CLOVER_AUDIO_AUDIODEVICE_HPP

#include "build.hpp"

namespace clover {
namespace audio {

class OutputStream;

/// Outputs audio data from OutputStream
class AudioDevice {
public:
	AudioDevice();
	virtual ~AudioDevice();

	virtual OutputStream& getOutputStream()= 0;
};

extern AudioDevice* gAudioDevice;
	
} // audio
} // clover

#endif // CLOVER_AUDIO_AUDIODEVICE_HPP