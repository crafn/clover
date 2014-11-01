#ifndef CLOVER_UI_EDITOR_EC_UI_PERFORMANCE_HPP
#define CLOVER_UI_EDITOR_EC_UI_PERFORMANCE_HPP

#include "../editorcomponent_ui.hpp"
#include "build.hpp"
#include "game/editor/components/ec_performance.hpp"
#include "global/eventreceiver.hpp"
#include "gui/element_linegraph.hpp"
#include "util/string.hpp"
#include "util/hashmap.hpp"

namespace clover {
namespace ui { namespace game { namespace editor {

template <>
struct EditorComponentUiTraits<PerformanceEc> {
	static util::Coord defaultRadius(){ return util::Coord::VF(0.5); }
};

class PerformanceEcUi : public EditorComponentUi<PerformanceEc> {
public:
	typedef EditorComponentUi<PerformanceEc> BaseClass;
	
	PerformanceEcUi(PerformanceEc&);
	virtual ~PerformanceEcUi(){}
	
	virtual void onEvent(global::Event&);
	virtual void update();
	
protected:
	virtual void onResize();
	
private:
	void addToPerformanceGraph(const util::Str8& name, real64 value);

	util::HashMap<util::Str8, gui::LineGraphElement::NodeId> performanceGraphLineIdMap;
	gui::LineGraphElement performanceGraph;
};

}}} // ui::game::editor
} // clover

#endif // CLOVER_UI_EDITOR_EC_UI_PERFORMANCE_HPP