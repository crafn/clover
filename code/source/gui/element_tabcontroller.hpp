#ifndef CLOVER_GUI_TABCONTROLLER_HPP
#define CLOVER_GUI_TABCONTROLLER_HPP

#include "build.hpp"
#include "element_button.hpp"
#include "element.hpp"
#include "element_tab.hpp"
#include "element_layout_linear.hpp"

namespace clover {
namespace gui {

class TabControllerElement : public Element {
public:
	typedef Element BaseClass;
	
	/// rad and btn_height in same space
	TabControllerElement(const util::Coord& pos, const util::Coord& rad, const util::Coord& btn_height);
	virtual ~TabControllerElement();
	
	TabElement& addTab(const util::Str8& title);

	virtual void setMinRadius(const util::Coord& r);
	virtual void setMaxRadius(const util::Coord& r);

	virtual void postUpdate();

private:
	struct Tab {
		Tab(gui::ButtonElement&& b, TabElement&& t):
			tabButton(std::move(b)), tabElement(std::move(t)){}
		
		gui::ButtonElement tabButton;
		TabElement tabElement;
	};
	
	// Contains btnLayout and active tabElement
	gui::LinearLayoutElement layout;

	// Contains all tabButtons
	gui::LinearLayoutElement btnLayout;
	
	int32 activeTabIndex;
	util::DynArray<Tab> tabs;
	
	util::Coord btnHeight;
};

} // gui
} // clover

#endif // CLOVER_GUI_TABCONTROLLER_HPP