#ifndef CLOVER_GUI_PANEL_HPP
#define CLOVER_GUI_PANEL_HPP

#include "build.hpp"
#include "element.hpp"
#include "elementvisualentity.hpp"

namespace clover {
namespace gui {

class PanelElement : public Element {
public:	
	PanelElement(const util::Coord& offset= util::Coord::VF(0), const util::Coord& rad= util::Coord::VF(0));
	PanelElement(PanelElement&&)= default;
	virtual ~PanelElement();
	
	// No graphics
	void setBgInvisible(){ /*autoREs.clear();*/ }
	
	virtual void spatialUpdate() override;
	
protected:
	ElementVisualEntity* visual;
};

} // gui
} // clover

#endif // CLOVER_GUI_PANEL_HPP