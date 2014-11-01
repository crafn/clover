#ifndef CLOVER_GUI_2DCONTROLLER_HPP
#define CLOVER_GUI_2DCONTROLLER_HPP

#include "build.hpp"
#include "element.hpp"
#include "element_button.hpp"
#include "visual/entity_def_model.hpp"

namespace clover {
namespace gui {

/// 2d area with moveable point
class Controller2dElement : public Element {
public:
	Controller2dElement(const util::Coord& pos, const util::Coord& rad);
	virtual ~Controller2dElement();

	virtual void postUpdate();

	/// @return [(-1,-1), (1,1)]
	util::Vec2d getValue(){ return value; }
	
	void setCircleBoundary(bool b=true){ circleBoundary= b; }

protected:
	void updateRE();

	util::Vec2d value;
	bool circleBoundary;
	
	//visual::ModelEntity* bgRE;
	//visual::ModelEntity* pointRE;
};

} // gui
} // clover

#endif // CLOVER_GUI_2DCONTROLLER_HPP