#include "devlogic.hpp"
#include "debug/debugprint.hpp"
#include "global/cfg_mgr.hpp"
#include "global/env.hpp"
#include "global/event.hpp"
#include "game/worldchunk.hpp"
#include "game/worldchunk_mgr.hpp"
#include "game/worldentity_mgr.hpp"
#include "game/worldentity_handleconnection.hpp"
#include "audio/audio_mgr.hpp"
#include "visual/entity_mgr.hpp"
#include "visual/shader_mgr.hpp"
#include "visual/visual_mgr.hpp"
#include "physics/phys_mgr.hpp"
#include "resources/cache.hpp"
#include "basegamelogic.hpp"
#include "ingamelogic.hpp"
#include "hardware/device.hpp"

#include <fstream>

namespace clover {
namespace game {

util::Str8 stringifyProfilingResult(util::Profiler::Result result, std::thread::id thread){
	util::Str8 str;
	str += util::Str8::format("--- Profiling results ---\n%u measurements\n\n",
			(uint32)result.getMeasureCount());

	for (auto item : result.getSortedLabels(thread))
		str += util::Str8::format("%.4f: \t %s\n", item.share, item.id.c_str());
	str += "\n";

	for (auto item : result.getSortedSamples(thread)){
		str += util::Str8::format("%.4f - %i/%i: %s\n",
				item.share,
				(int)item.exclusiveMemAllocs,
				(int)item.inclusiveMemAllocs,
				item.id.c_str());
	}

	return str;
}

void profileFor(double time){
	std::thread::id thread= std::this_thread::get_id();
	std::thread t([time, thread] (){
		util::Profiler profiler(global::gCfgMgr->get("dev::profilerSampleRate", 2.0));
		std::this_thread::sleep_for(
			std::chrono::milliseconds((int)(time*1000.0)));

		util::Str8 results= stringifyProfilingResult(
			profiler.popResult(global::gCfgMgr->get("dev::profilerMinShare", 0.01)),
			thread);

		print(debug::Ch::Dev, debug::Vb::Trivial, "%s", results.cStr());

		std::ofstream f("last_profiling.txt", std::ios::binary);
		f.write(results.cStr(), results.sizeBytes());
	});
	t.detach();
}

DevLogic::DevLogic()
	: profiler(global::gCfgMgr->get("dev::profilerSampleRate", 2.0))
	, fpsFrameCount(0)
	, fpsTimer(0)
	, fpsPrintFilter(0){
}

void DevLogic::update(){
	PROFILE();

	debug::gDebugPrint.updateBuffer();
	
	// SubSystem info update

	if (fpsTimer > global::gCfgMgr->get<real32>("dev::perfMeasureInterval")) {

		// Print some statistics about subsystems
		if (fpsPrintFilter == 0){
			print(debug::Ch::General, debug::Vb::Moderate, "--- Tick ---");
			print(debug::Ch::General, debug::Vb::Moderate, "FPS: %f", fpsFrameCount/fpsTimer);
			print(debug::Ch::General, debug::Vb::Moderate, "Chunk count:		%i",
					(int)global::g_env->worldMgr->getChunkMgr().getChunkCount());
			print(debug::Ch::General, debug::Vb::Moderate, "WorldEntity count:	%i",
					(int)global::g_env->worldMgr->getWeMgr().getEntityCount());
			print(debug::Ch::General, debug::Vb::Moderate,
					"visual::ModelEntity count: %i,	  shader count: %i",
					(int)visual::gVisualMgr->getEntityMgr().getModelEntityCount(),
					(int)visual::gVisualMgr->getShaderMgr().getShaderCount());
			print(debug::Ch::General, debug::Vb::Moderate, "PhysObject count:	%i", global::g_env->physMgr->calcObjectCount());
			print(debug::Ch::General, debug::Vb::Moderate, "SoundInstance count:  %lu", (unsigned long)global::g_env->audioMgr->getSoundInstanceCount());
			print(debug::Ch::General, debug::Vb::Moderate, "AudioSourceInstance count:	%lu", (unsigned long)global::g_env->audioMgr->getAudioSourceCount());
			print(debug::Ch::General, debug::Vb::Moderate, "Free audio channel count:  %lu", (unsigned long)global::g_env->audioMgr->getFreeChannelCount());
			print(debug::Ch::General, debug::Vb::Trivial, "Viewport: %i, %i", hardware::gDevice->getViewportSize().x, hardware::gDevice->getViewportSize().y);

			print(debug::Ch::General, debug::Vb::Trivial, "Timers");
		}

		uint32 lost_handle_count= game::WeHandle::countLostHandles();
		uint32 strict_handle_count= StrictHandleConnection::countConnections();

		util::Profiler::Result profiling_result=
			profiler.popResult(global::gCfgMgr->get("dev::profilerMinShare", 0.01));
		performanceTimerResults.clear();
		for (auto& item : profiling_result.getSortedLabels(std::this_thread::get_id())){
			real32 average_time= (real32)item.share*fpsTimer/fpsFrameCount;
			real32 share= (real32)item.share;
			performanceTimerResults.pushBack({item.id, average_time, share});

			if (fpsPrintFilter == 0){
				util::Str8 s= performanceTimerResults.back().name;
				s.resize(20, ' ');

				print(debug::Ch::General,
							debug::Vb::Moderate,
							"\t%s: \t%f \t%.2f%%",
							s.cStr(),
							performanceTimerResults.back().averageTime,
							performanceTimerResults.back().percentage*100);
			}
		}

		++fpsPrintFilter;
		fpsPrintFilter= fpsPrintFilter%20;

		fpsFrameCount= 0;
		fpsTimer= 0;

		global::Event e(global::Event::OnPerformanceTimersUpdate);
		e.send();

	}

	++fpsFrameCount;
	fpsTimer += util::gRealClock->getDeltaTime();
	
	editor.update();
	
}

} // game
} // clover
