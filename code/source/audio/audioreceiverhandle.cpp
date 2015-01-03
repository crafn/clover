#include "audioreceiverhandle.hpp"
#include "audio_mgr.hpp"
#include "global/env.hpp"

namespace clover {
namespace audio {

AudioReceiverHandle::AudioReceiverHandle(AudioReceiver* recv)
		: audioReceiver(recv){

	if (audioReceiver)
		listenForEvent(global::Event::OnAudioReceiverDestroy);
}
	
AudioReceiverHandle::~AudioReceiverHandle(){
}

void AudioReceiverHandle::onEvent(global::Event& e){
	
	switch(e.getType()){
		case global::Event::OnAudioReceiverDestroy:
			if (e(global::Event::Object).getPtr<AudioReceiver>() == audioReceiver.get()){
				audioReceiver= 0;
				unlistenForEvent(global::Event::OnAudioReceiverDestroy);
			}
		break;
		
		default: break;
	}
	
}

void AudioReceiverHandle::assignNewReceiver(){
	if (!audioReceiver)
		listenForEvent(global::Event::OnAudioReceiverDestroy);
	
	operator=(global::g_env->audioMgr->createAudioReceiver());
	
	ensure(audioReceiver);
}

void AudioReceiverHandle::setPosition(const util::Vec2d& pos){
	ensure(audioReceiver);
	
	audioReceiver->setPosition(pos);
}

} // audio
} // clover
