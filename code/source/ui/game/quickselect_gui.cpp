#include "quickselect_gui.hpp"
#include "gui/element_button.hpp"

namespace clover {
namespace ui { namespace game {

QuickSelectRingGui::QuickSelectRingGui()
		: Element(util::Coord::VF(0), util::Coord::VF(0.5)){
	touchable= false;

	for (uint32 i=0; i<Layout_Last; ++i){
		real64 radius=	0.13*(i+1);
		layouts.pushBack(gui::RadialLayoutElement(util::Coord::VF(0), util::Coord::VF(radius)));

		util::Str8 text= "default";
		switch (i){
			case Layout_Secondary: text= "Secondary"; break;
			case Layout_Combat: text= "Combat"; break;
			case Layout_Tools: text= "Tools"; break;
			case Layout_Building: text= "Building"; break;
			case Layout_Misc: text= "Misc"; break;
			default: ensure(0);
		}

		labels.pushBack(gui::TextLabelElement(text, util::Coord::VF({0, radius+0.05})));

		addSubElement(labels.back());
		addSubElement(layouts.back());
	}

	selectedEntity= -1;
}

QuickSelectRingGui::~QuickSelectRingGui(){
}

void QuickSelectRingGui::add(game::WeHandle h){
	icons.pushBack(Icon(h));
	layouts[Layout_Building].addSubElement(icons.back().getGuiElement());
}

void QuickSelectRingGui::remove(game::WeHandle h){
	for (auto it= icons.begin(); it!= icons.end(); ++it){
		//print(debug::Ch::Gui, debug::Vb::Trivial, "remove(..): before: guiElement= %x", &it->getGuiElement());
	}

	//int32 i=0;
	for (auto it= icons.begin(); it!= icons.end(); ++it){
		if (it->getGuiElement().getHandle() == h){
			//print(debug::Ch::Gui, debug::Vb::Trivial, "erase: %i, size: %i", i, icons.size());
			icons.erase(it);
			//print(debug::Ch::Gui, debug::Vb::Trivial, "size after: %i", icons.size());
			break;
		}
		//++i;
	}

	for (auto it= icons.begin(); it!= icons.end(); ++it){
		//print(debug::Ch::Gui, debug::Vb::Trivial, "after(..): before: guiElement= %x", &it->getGuiElement());
	}
}

gui::WeIconElement* QuickSelectRingGui::getTriggeredIcon(){
	//int32 i=0;
	for (auto &m : icons){
		if (m.getGuiElement().isSelectTriggered()){
			//print(debug::Ch::Gui, debug::Vb::Trivial, "getTriggeredIcon() i: %i", i);
			return &m.getGuiElement();
		}
		//++i;
	}
	return 0;
}

gui::WeIconElement* QuickSelectRingGui::getNextSelection(bool next){
	int32 sel= selectedEntity;

	if (next){
		++sel;
	}
	else{
		--sel;
	}

	if (sel < -1) sel = icons.size()-1;
	if (sel >= (int32)icons.size()) sel = -1;

	//print(debug::Ch::Gui, debug::Vb::Trivial, "sel: %i, size: %i", sel, icons.size());

	// Arvo -1 tarkoittaa, että kädessä ei ole mitään
	if (sel == -1) return 0;

	//print(debug::Ch::Gui, debug::Vb::Trivial, "getSelection sel: %i", sel);

	return &icons[sel].getGuiElement();
}
void QuickSelectRingGui::setSelection(game::WeHandle h){
	if (h){
		for (uint32 i=0; i<icons.size(); ++i){
			if (icons[i].getGuiElement().getHandle() == h){
				selectedEntity= i;
				return;
			}
		}
	}
	selectedEntity= -1;
}

void QuickSelectRingGui::toggle(){
	if (isActive()){
		setActive(false);
	}
	else {
		setActive(true);
	}
}

}} // ui::game
} // clover