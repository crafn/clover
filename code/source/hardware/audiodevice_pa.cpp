#include "audiodevice_pa.hpp"
#include "debug/debugprint.hpp"
#include "global/cfg_mgr.hpp"
#include "util/dyn_array.hpp"
#include "global/exception.hpp"
#include "util/string.hpp"

/// @todo Use util::Atomic
#include <boost/atomic.hpp>

namespace clover {
namespace hardware {

//
// PaOutputStream
//

PaOutputStream::PaOutputStream(const PaStreamParameters& output_params, uint32 sample_rate, SizeType buffer_size)
		: paStream(nullptr)
		, stream(channelCount, buffer_size)
		, feed(stream.getDeviceAudioFeed()){
	PaStreamFlags flags= paNoFlag;

	ensure(output_params.device != -1);
	PaError err = Pa_OpenStream(
			  &paStream,
			  NULL,
			  &output_params,
			  sample_rate,
			  buffer_size/4, // Update 1/4 of buffer at once
			  flags,
			  &PaOutputStream::audioCallback,
			  (void*)&feed);


	if(err != paNoError)
		throw global::Exception("PaOutputStream(): PortAudio stream open failed: %s", Pa_GetErrorText(err));

	err= Pa_StartStream(paStream);

	if(err != paNoError)
		throw global::Exception("PaOutputStream(): PortAudio stream start failed: %s", Pa_GetErrorText(err));
}

PaOutputStream::~PaOutputStream(){
	PaError err = Pa_StopStream(paStream);

	if(err != paNoError)
		print(debug::Ch::Audio, debug::Vb::Critical, "PaOutputStream::~PaOutputStream(): PortAudio stream stop failed: %s",	 Pa_GetErrorText(err));

	err = Pa_CloseStream(paStream);
	if(err != paNoError)
		print(debug::Ch::Audio, debug::Vb::Critical, "AudioDevice::shutdown(): PortAudio stream close failed: %s",	Pa_GetErrorText(err));
}

void output(audio::DeviceAudioFeed::Channel& channel, SizeType output_frame_count, real32* output_buf){
	SizeType samples_waiting= channel.getSampleCount();
	SizeType read_count= output_frame_count; // Two channels

	if (read_count > samples_waiting)
		read_count= samples_waiting;

	util::DynArray<audio::DeviceAudioFeed::Channel::SampleChunk> sample_chunks;
	sample_chunks= channel.readSamples(read_count);

	for (const auto& samples : sample_chunks){

		if (channel.didUnderrun()){
			print(debug::Ch::General, debug::Vb::Trivial, "Audio buffer underrun (increase buffer size)");
		}

		for (SizeType i= 0; i < samples.size(); ++i){
			ensure(i < output_frame_count);
			real32 amplitude;

			amplitude= samples[i].amplitude;

			real32 pan_vol_mul[2]= {	util::limited(-samples[i].pan + 1.0f, 0.0f, 1.0f),
										util::limited(samples[i].pan + 1.0f, 0.0f, 1.0f) };

			output_buf[2*i] += amplitude*pan_vol_mul[0];
			output_buf[2*i+1] += amplitude*pan_vol_mul[1];

			if (util::abs(output_buf[2*i]) > 1.0 || util::abs(output_buf[2*i+1]) > 1.0)
					print(debug::Ch::Audio, debug::Vb::Moderate, "Audio clipped!");
		}
	}
}

int32 PaOutputStream::audioCallback(	const void* input_buffer, void* output_buffer,
										unsigned long output_frame_count,
										const PaStreamCallbackTimeInfo* time_info,
										PaStreamCallbackFlags status_flags,
										void* user_data){

	audio::DeviceAudioFeed& feed= *reinterpret_cast<audio::DeviceAudioFeed*>(user_data);
	real32* output_buf= reinterpret_cast<real32*>(output_buffer);

	for (SizeType i=0; i<output_frame_count; ++i){
		output_buf[2*i]= 0.0f;
		output_buf[2*i+1]= 0.0f;
	}

	for (SizeType i=0; i<feed.getChannelCount(); ++i){
		audio::DeviceAudioFeed::Channel channel= feed.getChannel(i);

		if (!channel.isActive())
			continue; // Channel not playing

		if (channel.eos()){
			channel.close();
			continue;
		}

		output(channel, output_frame_count, output_buf);
	}
	return 0;
}

//
// PaAudioDevice
//

PaAudioDevice::PaAudioDevice():
	defaultDeviceDId(-1){
	chooseWiselyAndCreateNicely();

	paOutputStream= std::move(std::unique_ptr<PaOutputStream>(
		new PaOutputStream(
			getDefaultStreamParameters(),
			getDefaultSampleRate(),
			getDefaultBufferSize()
		)
	));
}

PaAudioDevice::~PaAudioDevice(){
	paOutputStream.reset();
	shutdown();
}

void PaAudioDevice::chooseWiselyAndCreateNicely(){
	PaError err= Pa_Initialize();

	if(err != paNoError)
		throw global::Exception("AudioDevice::chooseWiselyAndCreateNicely(): PortAudio init failed: %s",  Pa_GetErrorText( err ));

	// Look for all audio devices
	int32 num_devices;

	num_devices = Pa_GetDeviceCount();
	if( num_devices < 0 ) {
		throw global::Exception("AudioDevice::chooseWiselyAndCreateNicely(): Pa_GetDeviceCount failed: %s", Pa_GetErrorText(num_devices));

	} else if (num_devices == 0) {
		print(debug::Ch::Audio, debug::Vb::Critical, "No audio devices");
		return;
	} else {

		print(debug::Ch::Audio, debug::Vb::Trivial, "Available audio devices:");
		const PaDeviceInfo *device_info;

		for(int32 i=0; i<num_devices; i++) {

			device_info = Pa_GetDeviceInfo(i);

			print(debug::Ch::Audio, debug::Vb::Trivial, "	 %s", device_info->name);
			print(debug::Ch::Audio, debug::Vb::Trivial, "		 Low latency: %f", device_info->defaultLowOutputLatency);
			print(debug::Ch::Audio, debug::Vb::Trivial, "		 High latency: %f", device_info->defaultHighOutputLatency);

		}

		util::DynArray<int32> device_ids;
		defaultDeviceDId= -1;

		PaStreamParameters output_params= getDefaultStreamParameters();

		util::DynArray<util::Str8> preferred;
		util::Str8 user_preference= global::gCfgMgr->get<util::Str8>("hardware::audioDevice", "");
		if (!user_preference.empty())
			preferred.pushBack(user_preference);

		preferred.pushBack("default");
		preferred.pushBack("Microsoft Sound Mapper - Output");

		// Find supported devices
		for (int32 i=0; i<num_devices; i++) {
			device_info= Pa_GetDeviceInfo(i);
			output_params.device= i;

			err = Pa_IsFormatSupported(0, &output_params, defaultSampleRate);
			if(err == paFormatIsSupported) {
				device_ids.pushBack(i);
			}
		}

		// Find preferred out of supported
		for (auto& m : preferred){
			auto it=
				std::find_if(device_ids.begin(), device_ids.end(),
					[&m](int32 id){
						if(util::Str8(Pa_GetDeviceInfo(id)->name).lowerCased() == m.lowerCased())
							return true;
						return false;
					}
				);

			if (it != device_ids.end()){
				defaultDeviceDId= *it;
				break;
			}
		}

		// Pick first if no match
		if (defaultDeviceDId == -1){
			for (int32 i : device_ids){
				defaultDeviceDId= i;
				break;
			}
		}

		// Error if no supported devices
		if (defaultDeviceDId == -1)
			throw global::Exception("Sufficient audio device not found");

		device_info= Pa_GetDeviceInfo(defaultDeviceDId);

		print(debug::Ch::Audio, debug::Vb::Trivial, "-> %s\n	With suggested latency: %ims",
					device_info->name,
					(int)(output_params.suggestedLatency*1000));
	}
}

void PaAudioDevice::shutdown(){

	PaError err = Pa_Terminate();
	if(err != paNoError)
		print(debug::Ch::Audio, debug::Vb::Critical, "AudioDevice::shutdown(): PortAudio terminate failed: %s",	 Pa_GetErrorText( err ));
}

SizeType PaAudioDevice::getDefaultBufferSize() const {
	return global::gCfgMgr->get<SizeType>("audio::audioBufferSize", 2048);
}

PaStreamParameters PaAudioDevice::getDefaultStreamParameters() const {
	PaStreamParameters params;
	params.channelCount = 2;
	params.device= defaultDeviceDId;
	params.hostApiSpecificStreamInfo= nullptr;
	params.sampleFormat= paFloat32;
	params.suggestedLatency= 0.05;
	return params;
}

} // hardware
} // clover