#include "audiodevice.hpp"

namespace clover {
namespace audio {

AudioDevice* gAudioDevice;

AudioDevice::AudioDevice(){
	if (!gAudioDevice)
		gAudioDevice= this;
}

AudioDevice::~AudioDevice(){
	if (gAudioDevice == this)
		gAudioDevice= nullptr;
}
	
} // audio
} // clover