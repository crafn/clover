#ifndef CLOVER_HARDWARE_AUDIODEVICE_PA_HPP
#define CLOVER_HARDWARE_AUDIODEVICE_PA_HPP

#include "audio/audiodevice.hpp"
#include "audio/outputstream.hpp"
#include "build.hpp"
#include "util/dyn_array.hpp"

#include <portaudio.h>

namespace clover {
namespace hardware {

/// Handles streaming of audio::OutputStream to portaudio
class PaOutputStream {
public:
	PaOutputStream(const PaStreamParameters& output_params, uint32 sample_rate, SizeType buffer_size);
	virtual ~PaOutputStream();

	static constexpr uint32 channelCount= 32;

	audio::OutputStream& getOutputStream(){ return stream; }

private:
	PaStream* paStream;
	audio::OutputStream stream;
	audio::DeviceAudioFeed feed;

	static int32 audioCallback(  	const void* input_buffer, void* output_buffer,
									unsigned long sample_count,
									const PaStreamCallbackTimeInfo* time_info,
									PaStreamCallbackFlags status_flags,
									void* user_data);
};


/// Audio device which uses PortAudio
class PaAudioDevice : public audio::AudioDevice {
public:
	PaAudioDevice();
	virtual ~PaAudioDevice();

	audio::OutputStream& getOutputStream(){ return paOutputStream->getOutputStream(); }

private:
	void chooseWiselyAndCreateNicely();
	void shutdown();

	uint32 getDefaultDeviceDId() const { return defaultDeviceDId; }
	SizeType getDefaultBufferSize() const;
	uint32 getDefaultSampleRate() const { return 44100; }
	PaStreamParameters getDefaultStreamParameters() const;

	static constexpr uint32 defaultSampleRate= 44100;

	std::unique_ptr<PaOutputStream> paOutputStream;
	int32 defaultDeviceDId;
};

} // hardware
} // clover

#endif // CLOVER_HARDWARE_AUDIODEVICE_PA_HPP