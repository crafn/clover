#include "global/env.hpp"
#include "gui_mgr.hpp"
#include "element.hpp"
#include "util/profiling.hpp"

namespace clover {
namespace gui {

GuiMgr::GuiMgr(){
	if (!global::g_env.guiMgr)
		global::g_env.guiMgr= this;

	Element::guiCursor= &guiCursor;
	audioSource.assignNewSource(audio::AudioSource::Type::Global);
	Element::audioSource= audioSource;

	listenForEvent(global::Event::OnGuiCallbackTrigger);
}

GuiMgr::~GuiMgr(){
	accumulatedCallbacks.clear();
	BaseElementVisualEntity::clearAll();
	
	ensure_msg(BaseElementVisualEntity::getEntityCount() == 0,
		"Gui visuals not destroyed: %i", BaseElementVisualEntity::getEntityCount());
	
	Element::audioSource= audio::AudioSourceHandle();

	if (global::g_env.guiMgr == this)
		global::g_env.guiMgr= nullptr;
}

void GuiMgr::onEvent(global::Event& e){
	switch(e.getType()){
		
		case global::Event::OnGuiCallbackTrigger: {
			accumulatedCallbacks.pushBack(e(global::Event::Object).getPtr<BaseCallback>());
		}
		break;
		
		default: break;
	}
}

void GuiMgr::update(){
	PROFILE_("gui");
	
	guiCursor.preUpdate();
	
	for (auto& m : Element::elementTable){
		if (m && m->getSuperElement() == nullptr){
			m->recursiveSpatialUpdate();
		}
	}
	
	int32 depthIndex= 0;
	for (int32 i=Element::elementTable.size()-1; i>=0; --i){
		
		if (Element::elementTable[i] == 0) continue;
		
		Element& element= *Element::elementTable[i];
		
		// Päivitetään parentittomat elementit, ne päivittävät omat jäsenensä
		if (element.getSuperElement() == 0){
			element.preUpdate();
			element.postUpdate();
		}
	}
	
	// Fire accumulated callbacks
	for (uint32 i=0; i<accumulatedCallbacks.size(); ++i){
		accumulatedCallbacks[i]->call();
	}
	
	accumulatedCallbacks.clear();
	
	// Update positions because callbacks might have changed something
	// If too heavy, flag spatial changes and call only when needed
	// (Actually needed only for layouts to update on the same frame when a subElement changes size in editor)
	for (auto& m : Element::elementTable){
		
		if (m && m->getSuperElement() == nullptr){
			m->recursiveSpatialUpdate();
			m->depthOrderUpdate(depthIndex);
		}
	}
	
	BaseElementVisualEntity::commonUpdate();
	
	guiCursor.postUpdate();
}

void GuiMgr::destroyTree(Element& e){
	destroySubTree(e);
	delete &e;
}

void GuiMgr::destroySubTree(Element& e){
	while (!e.getSubElements().empty()){
		destroySubTree(*e.getSubElements()[0]);
		delete e.getSubElements()[0];
	}
}

Cursor& GuiMgr::getCursor(){
	return guiCursor;
}

void GuiMgr::onDestroy(const Element& e){
	// Remove callback if owner is destroyed
	for (auto it= accumulatedCallbacks.begin(); it != accumulatedCallbacks.end();){
		if (&(*it)->getOwner() == &e){
			it= accumulatedCallbacks.erase(it);
		}
		else
			++it;
	}
}

} // gui
} // clover
