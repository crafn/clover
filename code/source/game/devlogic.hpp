#ifndef CLOVER_DEVLOGIC_HPP
#define CLOVER_DEVLOGIC_HPP

#include "build.hpp"
#include "game/editor/editor.hpp"
#include "ui/hid/actionlistener.hpp"
#include "util/dyn_array.hpp"
#include "util/profiler.hpp"
#include "util/string.hpp"

namespace clover {
namespace game {

/// Handles developer-only logic which can be cut out for release
class DevLogic {
public:
	struct PerformanceTimerResult {
		/// Name of timer
		util::Str8 name;
		/// In seconds
		real32 averageTime;
		/// Scale 0-1
		real32 percentage;
	};

	DevLogic();
	void update();

	const util::DynArray<PerformanceTimerResult>& getLastPerformanceTimerResults() const { return performanceTimerResults; }

	editor::Editor& getEditor(){ return editor; }

private:
	util::Profiler profiler;

	int32 fpsFrameCount;
	real32 fpsTimer;
	int32 fpsPrintFilter;

	util::DynArray<PerformanceTimerResult> performanceTimerResults;

	editor::Editor editor;
	ui::hid::ActionListener<> profileListener;
};

} // game
} // clover

#endif // CLOVER_DEVLOGIC_HPP
