#include "audiodevice_dummy.hpp"
#include "global/cfg_mgr.hpp"
#include "hardware/device.hpp"

namespace clover {
namespace audio {

DummyAudioDevice::DummyAudioDevice()
	: outputStream(32, global::g_env->cfg->get<SizeType>("audio::audioBufferSize", 2048))
	, runSimulatorThread(true)
	, outputSimulatorThread(
		std::thread(std::bind(&DummyAudioDevice::outputSimulator, this)))
{
}

DummyAudioDevice::~DummyAudioDevice()
{
	runSimulatorThread= false;
	outputSimulatorThread.join();
}

void DummyAudioDevice::outputSimulator()
{
	DeviceAudioFeed feed= outputStream.getDeviceAudioFeed();
	
	while (runSimulatorThread) {
		
		// Read (and discard) data from channels
		for (SizeType i=0; i<feed.getChannelCount(); ++i) {
			DeviceAudioFeed::Channel channel= feed.getChannel(i);

			if (!channel.isActive())
				continue;
			
			if (channel.eos()) {
				channel.close();
				continue;
			}
			
			// Read to prevent buffer overflowing
			channel.readSamples(channel.getSampleCount());
		}
		
		// Don't exhaust CPU
		global::g_env->device->sleep(0.01);
	}
}

} // audio
} // clover
