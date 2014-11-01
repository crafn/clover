#include "element_tabcontroller.hpp"

namespace clover {
namespace gui {

TabControllerElement::TabControllerElement(const util::Coord& pos, const util::Coord& rad, const util::Coord& btn_height):
	Element(pos, rad),
	layout(gui::LinearLayoutElement::Vertical, pos, rad),
	btnLayout(gui::LinearLayoutElement::Horizontal, pos, util::Coord({rad.x, btn_height.y}, rad.getType())),
	activeTabIndex(-1),
	btnHeight(btn_height){
	
	ensure(rad.getType() == btn_height.getType());
		
	addSubElement(layout);
	
	layout.setMargin(false);
	layout.mirrorFirstNodeSide(true);
	layout.addNode(btnLayout, 1);
}

TabControllerElement::~TabControllerElement(){
}

TabElement& TabControllerElement::addTab(const util::Str8& title){
	Tab t= Tab(	gui::ButtonElement	(	title, 
										btnLayout.getPosition(), 
										util::Coord(	{radius.x*0.3, btnHeight.y}, 
												radius.getType())	
										),
				
				TabElement				(	position, 
											util::Coord(	{radius.x, radius.y-btnHeight.y}, 
													radius.getType())
										)
				);
	t.tabElement.setBgInvisible();
	
	btnLayout.addSubElement(t.tabButton);
	
	// First tab is active by default
	if (tabs.empty()){
		layout.addNode(t.tabElement, 0);
		activeTabIndex= 0;
	}
	else
		t.tabElement.setActive(false);
	
	tabs.pushBack(std::move(t));
	
	return tabs.back().tabElement;
}

void TabControllerElement::setMinRadius(const util::Coord& r){
	BaseClass::setMinRadius(r);
	for (auto &m : tabs){
		m.tabElement.setMinRadius(r);
	}
}

void TabControllerElement::setMaxRadius(const util::Coord& r){
	BaseClass::setMaxRadius(r);
	for (auto &m : tabs){
		m.tabElement.setMaxRadius(r);
	}
}

void TabControllerElement::postUpdate(){
	BaseClass::postUpdate();
	
	// If a tab button is pressed, 
	for (uint32 i=0; i<tabs.size(); ++i){
		
		if ((int32)i != activeTabIndex && tabs[i].tabButton.isTriggered()){
			ensure(activeTabIndex >= 0 && activeTabIndex < (int32)tabs.size());
			
			// Deactivate previous
			layout.removeSubElement(tabs[activeTabIndex].tabElement);
			tabs[activeTabIndex].tabElement.setActive(false);
			
			activeTabIndex= (int32)i;
			
			// Activate pressed
			layout.addNode(tabs[activeTabIndex].tabElement, 0);
			tabs[activeTabIndex].tabElement.setActive();
			
			break;
		}
	}
}

} // gui
} // clover