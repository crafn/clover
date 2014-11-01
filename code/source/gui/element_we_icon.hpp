#ifndef CLOVER_GUI_WORLDENTITYICON_HPP
#define CLOVER_GUI_WORLDENTITYICON_HPP

#include "build.hpp"
#include "gui/element.hpp"
#include "visual/entity_def_model.hpp"
#include "game/worldentity_handle.hpp"
#include "gui/element_textlabel.hpp"

namespace clover {
namespace gui {

/// @todo This shouldn't be part of gui
class WeIconElement : public Element {
public:
	WeIconElement(game::WeHandle h, const util::Coord& pos=util::Coord::VF(0), const util::Coord& rad=util::Coord::VF(0.05));
	WeIconElement(WeIconElement&&);

	WeIconElement& operator=(const WeIconElement&)= delete;
	WeIconElement& operator=(WeIconElement&&)= delete;

	virtual ~WeIconElement();

	virtual void postUpdate() override;
	virtual void spatialUpdate() override;
	
	/// Triggeroitu valitsemistarkoituksessa (klikkaus)
	bool isSelectTriggered();

	game::WeHandle getHandle(){ return handle; }
	
private:
	void updateRE();
	
	game::WeHandle handle;
	
	visual::ModelEntityDef iconDef;
	CustomElementVisualEntity* elementVisual;
	
	bool selectTriggered;
};

} // gui
} // clover

#endif // CLOVER_GUI_WORLDENTITYICON_HPP