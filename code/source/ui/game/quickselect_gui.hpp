#ifndef CLOVER_UI_QUICKSELECT_HPP
#define CLOVER_UI_QUICKSELECT_HPP

#include "build.hpp"
#include "gui/element.hpp"
#include "gui/element_layout_radial.hpp"
#include "game/worldentity_handle.hpp"
#include "gui/element_we_icon.hpp"
#include "gui/element_button.hpp"

namespace clover {
namespace ui { namespace game {

using namespace clover::game;

class QuickSelectRingGui : public gui::Element {
public:
	QuickSelectRingGui();
	QuickSelectRingGui(const QuickSelectRingGui&)= delete;
	
	virtual ~QuickSelectRingGui();
	
	void add(game::WeHandle h);
	void remove(game::WeHandle h);
	
	gui::WeIconElement* getTriggeredIcon();
	gui::WeIconElement* getNextSelection(bool next=true);

	void setSelection(game::WeHandle h);
	void toggle();
	
private:
	enum Layout {
		Layout_Secondary,
		Layout_Combat,
		Layout_Tools,
		Layout_Building,
		Layout_Misc,
		Layout_Last
	};
	
	util::DynArray<gui::RadialLayoutElement> layouts;
	util::DynArray<gui::TextLabelElement> labels;
	
	struct Icon {
		Icon(const game::WeHandle& h): guiElement(new gui::WeIconElement(h)){
			
		}
	
		Icon(const Icon&)= delete;
		
		Icon(Icon&& other): guiElement(other.guiElement){
			other.guiElement= 0;
		}
		
		Icon& operator=(Icon&& other){
			if (other.guiElement == guiElement) return *this;
			delete guiElement;
			guiElement= other.guiElement;
			other.guiElement= 0;
			return *this;
		}
		
		~Icon(){
			delete guiElement;
			guiElement= 0;
		}
		
		gui::WeIconElement& getGuiElement() { return *guiElement; }
		
	private:
		gui::WeIconElement *guiElement;
	};
	
	util::DynArray<Icon> icons;
	int32 selectedEntity;
};

}} // ui::game
} // clover

#endif // CLOVER_UI_QUICKSELECT_HPP