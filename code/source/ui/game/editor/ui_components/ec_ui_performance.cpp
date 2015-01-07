#include "ec_ui_performance.hpp"
#include "global/cfg_mgr.hpp"
#include "global/env.hpp"
#include "game/basegamelogic.hpp"
#include "game/devlogic.hpp"
#include "util/time.hpp"

namespace clover {
namespace ui { namespace game { namespace editor {

PerformanceEcUi::PerformanceEcUi(PerformanceEc& comp):
	EditorComponentUi(comp),
	performanceGraph(util::Coord::VF(0), util::Coord::VF({0.5, 0.4})){
	
	performanceGraph.setMinRadius(util::Coord::VF(0.1));
		
	EditorComponentUi<PerformanceEc>::getContentElement().addSubElement(performanceGraph);
		
	listenForEvent(global::Event::OnPerformanceTimersUpdate);
}

void PerformanceEcUi::onEvent(global::Event& e){
	switch(e.getType()){
		case global::Event::OnPerformanceTimersUpdate: {
			auto results= global::g_env->gameLogic->getDevLogic()->getLastPerformanceTimerResults();

			for (auto& m : results)
				addToPerformanceGraph(m.name, m.percentage);
		}
		break;
		
		default: break;
	}
}

void PerformanceEcUi::update(){
	performanceGraph.clearBefore(global::g_env->realClock->getTime()-25);
	performanceGraph.setViewport(global::g_env->realClock->getTime()-20, global::g_env->realClock->getTime());
}

void PerformanceEcUi::onResize(){
	BaseClass::onResize();
	
	performanceGraph.setRadius(getContentElement().getRadius());
	performanceGraph.setMaxRadius(getContentElement().getRadius());
}

util::Color color(SizeType index){
	SizeType hue_steps= 7;
	SizeType saturation_steps= 2;
	SizeType lightness_steps= 2;
	SizeType color_count= hue_steps*saturation_steps*lightness_steps;
	
	index= index%color_count;

	SizeType hue_step= index%hue_steps;
	SizeType saturation_step= (index/hue_steps/lightness_steps)%saturation_steps;
	SizeType lightness_step= (index/hue_steps + 1)%lightness_steps;
	return util::Color::hsl({
			(real32)hue_step/(hue_steps),
			-(real32)saturation_step/(saturation_steps - 1)*0.5f + 1.0f,
			(real32)lightness_step/(lightness_steps - 1)*0.1f + 0.4f});
}

void PerformanceEcUi::addToPerformanceGraph(const util::Str8& name, real64 value){
	auto it= performanceGraphLineIdMap.find(name);
	gui::LineGraphElement::NodeId id;
	if (it == performanceGraphLineIdMap.end()){
		id=	 performanceGraph.createLine(color(performanceGraphLineIdMap.size()), name);
		performanceGraphLineIdMap[name]= id;
	}
	else {
		id= it->second;
	}

	performanceGraph.appendPoint(id, util::Vec2d{ global::g_env->realClock->getTime(), value });
}

}}} // ui::game::editor
} // clover
