#ifndef CLOVER_GUI_ELEMENT_FLOATING_HPP
#define CLOVER_GUI_ELEMENT_FLOATING_HPP

#include "build.hpp"
#include "element.hpp"

namespace clover {
namespace gui {

/// Invisible container element for floating
/// Doesn't go out of screen
/// Supposed to contain only one element (adjusts its radius by it)
class FloatingElement : public Element {
public:
	DECLARE_GUI_ELEMENT(FloatingElement)
	
	typedef Element Base;
	
	FloatingElement(const util::Coord& pos= util::Coord::VF(0));
	FloatingElement(FloatingElement&&)= default;
	virtual ~FloatingElement();
	
	/// setActive + position
	void show(const util::Coord& pos_hint);
	virtual void setOffset(const util::Coord& o) override;
	
	virtual void postUpdate() override;
	virtual void spatialUpdate() override;
	
	util::Coord getLimitedPosition(const util::Coord& pos);
	
	/// Called when clicked outside or pressed enter (enter should maybe be configurable option)
	GUI_CALLBACK(OnOutTrigger)
	
protected:
	
};

} // gui
} // clover

#endif // CLOVER_GUI_ELEMENT_FLOATING_HPP