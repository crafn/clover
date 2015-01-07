#include "element.hpp"
#include "debug/draw.hpp"
#include "global/env.hpp"
#include "gui_mgr.hpp"
#include "ui/userinput.hpp"
#include "visual/entity_def_model.hpp"

namespace clover {
namespace gui {

util::PtrTable<Element> Element::elementTable;
Cursor *Element::guiCursor= 0;
audio::AudioSourceHandle Element::audioSource;

Element::Element(const util::Coord& offset_, const util::Coord& rad):
					triggered(false),
					errorVisuals(false),
					elementType(None),
					active(true),
					visible(true),
					enabled(true),
					firstFrame(true),
					touchable(true),
					applySuperActivation(true),
					draggingType(DraggingType::Left),
					stretchable(false),
					autoResize(false),
					depthOffset(0),
					superElement(nullptr),
					curBbState(Bb_None),
					prevBbState(Bb_None){
	
	radius.setRelative();
	minRadius.setRelative();
	maxRadius.setRelative();
	
	radius.setType(util::Coord::View_Stretch);
	minRadius.setType(util::Coord::View_Stretch);
	maxRadius.setType(util::Coord::View_Stretch);
	offset.setType(util::Coord::View_Stretch);
	
	setRadius(rad);
	setMinRadius(rad);
	setMaxRadius(rad);
	setOffset(offset_);
	tableIndex= elementTable.insert(*this);
	
}

Element::Element(Element&& e):
		OnTrigger(std::move(e.OnTrigger)),
		OnSecondaryTrigger(std::move(e.OnSecondaryTrigger)),
		OnDraggingStart(std::move(e.OnDraggingStart)),
		OnDragging(std::move(e.OnDragging)),
		OnDraggingStop(std::move(e.OnDraggingStop)),
		triggered(e.triggered),
		errorVisuals(e.errorVisuals),
		elementType(e.elementType),
		active(e.active),
		visible(e.visible),
		enabled(e.enabled),
		firstFrame(e.firstFrame),
		touchable(e.touchable),
		applySuperActivation(e.applySuperActivation),
		draggingType(e.draggingType),
		stretchable(e.stretchable),
		autoResize(e.autoResize),
		depthOffset(0),
		position(e.position),
		offset(e.offset),
		radius(e.radius),
		minRadius(e.minRadius),
		maxRadius(e.maxRadius),
		superElement(0),
		curBbState(e.curBbState),
		prevBbState(e.prevBbState),
		tableIndex(e.tableIndex){
	elementTable[tableIndex]= this;
	e.tableIndex= -1;
	
	while(!e.visualEntities.empty()){
		e.visualEntities.front()->changeContainer(&visualEntities);
	}
	
	// Transfer subElements
	auto sub_elements= e.subElements;
	
	for (auto& m : sub_elements){
		e.removeSubElementImpl(*m);
	}
	
	for (auto& m : sub_elements){
		addSubElementImpl(*m);
	}
	
	// Switch this to superElement's subElement in the place of e
	if (e.superElement){
		e.superElement->onSubSwap(e, *this);
		
	}
	
}

Element& Element::operator=(Element&& e){
	OnTrigger= std::move(e.OnTrigger);
	OnSecondaryTrigger= std::move(e.OnSecondaryTrigger);
	OnDraggingStart= std::move(e.OnDraggingStart);
	OnDragging= std::move(e.OnDragging);
	OnDraggingStop= std::move(e.OnDraggingStop);
	triggered= e.triggered;
	errorVisuals= e.errorVisuals;
	elementType= e.elementType;
	active= e.active;
	visible= e.visible;
	enabled= e.enabled;
	firstFrame= e.firstFrame;
	touchable= e.touchable;
	applySuperActivation= e.applySuperActivation;
	stretchable= e.stretchable;
	autoResize= e.autoResize;
	depthOffset= e.depthOffset;
	draggingType= e.draggingType;
	position= e.position;
	offset= e.offset;
	radius= e.radius;
	minRadius= e.minRadius;
	maxRadius= e.maxRadius;
	curBbState= e.curBbState;
	prevBbState= e.prevBbState;
	
	destroyVisuals();
	
	if (superElement){
		superElement->removeSubElementImpl(*this);
	}
	
	elementTable.remove(tableIndex);
	elementTable[e.tableIndex]= this;
	tableIndex= e.tableIndex;
	e.tableIndex= -1;
	
	while(!e.visualEntities.empty()){
		e.visualEntities.front()->changeContainer(&visualEntities);
	}
	
	// Transfer subElements
	auto sub_elements= e.subElements;
	
	for (auto& m : sub_elements){
		e.removeSubElementImpl(*m);
	}
	
	for (auto& m : sub_elements){
		addSubElementImpl(*m);
	}
	
	if (e.superElement){
		e.superElement->onSubSwap(e, *this);
	}

	return *this;
}

Element::~Element(){
	destroyVisuals();
	
	for (auto m : subElements){
		m->setSuperElement(0);
	}
	
	if (superElement)
		superElement->onSubDestroy(*this);
		
	if (tableIndex >= 0)
		elementTable.remove(tableIndex);
	
	if (guiCursor->getDraggedElement() == this)
		guiCursor->stopDragging();
	
	if (guiCursor->getTouchedElement() == this)
		guiCursor->stopTouching();
	
	global::g_env->guiMgr->onDestroy(*this);
}

void Element::setOffset(const util::Coord& p){

	position.setType(p.getType());
	offset.setType(p.getType());
	
	offset= p;
	offset.setRelative();
	
	if (superElement)
		position= superElement->getPosition() + offset;
	else 
		position= offset;
		
	//recursiveOnSpatialChange();
}

void Element::setPosition(const util::Coord& p){
	if (superElement){
		setOffset(p - superElement->getPosition());
	}
	else {
		setOffset(p);
	}
}

void Element::setRadius(const util::Coord& r){

	minRadius.convertTo(r.getType());
	maxRadius.convertTo(r.getType());

	radius= r;
	radius.setRelative();
	
	if (maxRadius.x < r.x) maxRadius.x= r.x;
	if (maxRadius.y < r.y) maxRadius.y= r.y;
	
	if (minRadius.x > r.x) minRadius.x= r.x;
	if (minRadius.y > r.y) minRadius.y= r.y;
	
	ensure_msg(radius.x >= -util::epsilon && radius.y >= -util::epsilon, "%f, %f", radius.x, radius.y);
}

void Element::setMinRadius(const util::Coord& r){
	minRadius= r.converted(radius.getType());
	minRadius.setRelative();
		
	ensure_msg(minRadius.x >= -util::epsilon && minRadius.y >= -util::epsilon, "%f, %f", minRadius.x, minRadius.y);
	//ensure_msg(maxRadius.x >= minRadius.x && maxRadius.y >= minRadius.y, "min: %f, %f max: %f, %f", minRadius.x, minRadius.y, maxRadius.x, maxRadius.y);
}
	
void Element::setMaxRadius(const util::Coord& r){
	maxRadius= r.converted(radius.getType());
	maxRadius.setRelative();
	
	ensure_msg(maxRadius.x >= -util::epsilon && maxRadius.y >= -util::epsilon, "%f, %f", maxRadius.x, maxRadius.y);
	//ensure_msg(maxRadius.x >= minRadius.x && maxRadius.y >= minRadius.y, "min: %f, %f max: %f, %f", minRadius.x, minRadius.y, maxRadius.x, maxRadius.y);
}

void Element::minimizeRadius(){
	util::Coord min_radius(0.0, minRadius.getType());
			
	for (auto &m : subElements){
		
		const Element& e= *m;
		
		util::Coord cur_rad= m->getOffset().abs() + m->getRadius().abs();
		cur_rad= cur_rad.converted(min_radius.getType());
		
		if (cur_rad.x > min_radius.x)
			min_radius.x= cur_rad.x;
		if (cur_rad.y > min_radius.y)
			min_radius.y= cur_rad.y;
	}
	
	if (min_radius.x < minRadius.x) min_radius.x= minRadius.x;
	if (min_radius.y < minRadius.y) min_radius.y= minRadius.y;
	if (min_radius.x > maxRadius.x) min_radius.x= maxRadius.x;
	if (min_radius.y > maxRadius.y) min_radius.y= maxRadius.y;
	
	if (!stretchable){
		/// @todo Invariant ratio
	}
	
	radius= min_radius;
	radius.setRelative();
}

void Element::addSubElement(Element& element){
	addSubElementImpl(element);
}



void Element::removeSubElement(Element& element){
	removeSubElementImpl(element);
}

void Element::onSubSwap(Element& existing, Element& replacing){
	onSubDestroy(existing);
	addSubElement(replacing);
}



void Element::setActive(bool b){
	if (!b) {
		curBbState= Bb_None;
	}
	
	active= b;
	
	for (auto &m : visualEntities){
		m->setActiveTarget(active && visible);
	}
	
	for (auto m : subElements){
		if (m->applySuperActivation)
			m->setActive(b);
	}
	
}

void Element::onStopDragging(){
	OnDraggingStop(*this);
}

void Element::setVisible(bool b){
	visible= b;
	
	for (auto& m : visualEntities){
		m->setActiveTarget(active && visible);
	}
	
	for (auto& m : subElements){
		m->setVisible(visible);
	}
	
}

void Element::setEnabled(bool b){
	enabled= b;
	
	for (auto& m : visualEntities){
		m->setEnabledTarget(enabled);
	}
	
	for (auto& m : subElements){
		m->setEnabled(enabled);
	}
}

bool Element::recursiveIsTouched() const {
	for (auto& m : getSubElements()){
		if (m->recursiveIsTouched())
			return true;
	}
	return isTouched();
}

void Element::setSuperElement(Element* s){
	ensure(!superElement || !s);
	superElement= s;
}

void Element::onSubDestroy(Element& e){
	for (auto it= subElements.begin(); it!= subElements.end(); ++it){
		if (*it == &e){
			subElements.erase(it);
			break;
		}
	}
}


void Element::preUpdate(){
	firstFrame= false;

	if (isActive()){
		
		util::Color c= {0.0,0.0,1.0,0.8};
		
		if (elementType == LinearLayout)
			c= util::Color{0.0,0.0,0.0,0.8};
		
		global::g_env->debugDraw->addRect(position, radius, c);
		
	}
	else {
		curBbState= Bb_None;
	}

	for (auto m : subElements)
		m->preUpdate();

}



void Element::postUpdate(){
	triggered= false;
	if (!isActive()) return;
	
	// Last is on top
	for (auto it = subElements.rBegin(); it != subElements.rEnd(); ++it)
		(*it)->postUpdate();
	
	prevBbState= curBbState;
	
	if (touchable && isPointInside(guiCursor->getPosition())){
		
		if (guiCursor->canTouchElement(*this)){
			curBbState= Bb_Touch;
			guiCursor->touch(*this);
			
			if (enabled){
				if (gUserInput->isTriggered(UserInput::GuiCause)){
					triggered= true;
					OnTrigger(*this);
				}
				
				if (gUserInput->isTriggered(UserInput::GuiSecondaryCause)){
					OnSecondaryTrigger(*this);
				}
				
				if ((	(gUserInput->isTriggered(UserInput::GuiStartLeftDragging) && draggingType == DraggingType::Left) ||
						(gUserInput->isTriggered(UserInput::GuiStartMiddleDragging) && draggingType == DraggingType::Middle) ||
						(gUserInput->isTriggered(UserInput::GuiStartRightDragging) && draggingType == DraggingType::Right)) && 
						guiCursor->canStartDraggingElement(*this)){
					guiCursor->startDragging(*this);
					OnDraggingStart(*this);
				}
			}
		}
		else {
			curBbState= Bb_None;
		}
		
		if (guiCursor->canSetUnderElement(*this)){
			guiCursor->setUnderElement(*this);
		}
		

	}
	else {
		curBbState= Bb_None;
	}
	
	if (guiCursor->getDraggedElement() == this)
		OnDragging(*this);
	
}

void Element::depthOrderUpdate(int32& depth_index){
	depth_index += depthOffset;
	// Elements under subElements
	for (auto& m : visualEntities){
		if (!m->isOnTopOfElement())
			m->setDepth(depth_index);
		++depth_index;
	}
	
	for (auto& m : subElements){
		m->depthOrderUpdate(depth_index);
	}
	
	// Elements on top of subElements
	for (auto& m : visualEntities){
		if (m->isOnTopOfElement())
			m->setDepth(depth_index);
		++depth_index;
	}
	
	depth_index -= depthOffset;
}

bool Element::isPointInside(util::Coord p){
		p.convertTo(position.getType());
		util::Coord rad;
		rad.setRelative();
		rad= radius.converted(position.getType());
		
		if (p.x >= (position-rad).x && p.y >= (position-rad).y && 
			p.x < (position+rad).x && p.y < (position+rad).y )
			return true;

		return false;
}

bool Element::hasSubElement(const gui::Element& e) const {
	for (auto& m : subElements){
		if (m == &e) return true;
	}
	return false;
}

bool Element::recursiveHasSubElement(const gui::Element& e) const {
	if (hasSubElement(e)) return true;
	
	for (auto& m : subElements){
		if (m->recursiveHasSubElement(e)) return true;
	}
	
	return false;
}

void Element::bringTop(){
	if (superElement){
		superElement->bringTop(*this);
	}
	else {
		ensure_msg(0, "Not implemented");
	}
}

void Element::setErrorVisuals(bool b){
	errorVisuals= b;
	for (auto& m : visualEntities){
		m->setError(b);
	}
}

ElementVisualEntity* Element::createVisualEntity(ElementVisualEntity::Type t){
	return newVisualEntityConfig(new ElementVisualEntity(t, visualEntities));
}

TextElementVisualEntity* Element::createTextVisualEntity(TextElementVisualEntity::Type t){
	return newVisualEntityConfig(new TextElementVisualEntity(t, visualEntities));
}

CustomElementVisualEntity* Element::createCustomVisualEntity(){
	return newVisualEntityConfig(new CustomElementVisualEntity(visualEntities));
}

BaseElementVisualEntity* Element::newVisualEntityConfigImpl(BaseElementVisualEntity* e){
	e->setError(errorVisuals);
	return e;
}

void Element::destroyVisualEntity(BaseElementVisualEntity* entity){
	if (!entity) return;
	entity->changeContainer(nullptr);
}

void Element::destroyVisuals(){
	while(!visualEntities.empty()){
		destroyVisualEntity(visualEntities.back());
	}
}

void Element::bringTop(Element& sub_element){
	auto it= subElements.find(&sub_element);
	ensure(it != subElements.end());
	
	subElements.erase(it);
	subElements.pushBack(&sub_element);
}

void Element::spatialUpdate(){
	if (superElement)
		position= superElement->getPosition() + offset;
	else 
		position= offset;
	
	if (radius.x < minRadius.x){
		radius.x= minRadius.x;
	}
	if (radius.y < minRadius.y){
		radius.y= minRadius.y;
	}
	if (radius.x > maxRadius.x){
		radius.x= maxRadius.x;
	}
	if (radius.y > maxRadius.y){
		radius.y= maxRadius.y;
	}
	
	if (autoResize){
		minimizeRadius();
	}
}

void Element::recursiveSpatialUpdate(){
	if (!isActive()) return;

	spatialUpdate();
	
	// Update subElement radius'
	for (auto& m : subElements){
		m->recursiveSpatialUpdate();
	}
	
}

void Element::addSubElementImpl(Element& element){
	for (auto m : subElements){
		ensure(m != &element);
	}
	
	if (!isActive())
		element.setActive(false);
		
	subElements.pushBack(&element);
	element.setSuperElement(this);
	
}

void Element::removeSubElementImpl(Element& element){
	auto it= subElements.find(&element);

	if (it!= subElements.end()){
		subElements.erase(it);
		element.setSuperElement(0);
	}
}

} // gui
} // clover
