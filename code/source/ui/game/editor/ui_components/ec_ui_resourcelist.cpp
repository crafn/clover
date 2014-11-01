#include "ec_ui_resourcelist.hpp"

namespace clover {
namespace ui { namespace game { namespace editor {

ResourceListEcUi::ResourceListEcUi(ResourceListEc& comp):
	EditorComponentUi(comp),
	contentLayout(gui::LinearLayoutElement::Vertical, util::Coord::P(0), util::Coord::P(0.0)),
	searchField(gui::TextFieldElement::Type::Dev, util::Coord::P({0,20})),
	searchResultsList(util::Coord::P({0,-20}), util::Coord::P({200, 30})),
	searchGuiUpdateNeeded(false),
	maxResultCount(7){

	EditorComponentUi::getContentElement().addSubElement(contentLayout);
	contentLayout.addSubElement(searchField);
	contentLayout.addSubElement(searchResultsList);
	contentLayout.setMargin(false);
	
	searchField.setOnTextChangeCallback([=] (gui::TextFieldElement& e) {
		updateSearch(e.getText());
	});
	
	searchResultsList.setOnSelectCallback([=] (gui::ListBoxElement& e) {
		auto sel= e.getSelected();
		ensure(!sel.empty());
		ensure(!searchResults.empty());
		
		uint32 index= sel.front();
		ensure(index < searchResults.size());
		
		global::Event event(global::Event::OnEditorResourceSelect);
		event(global::Event::Resource)= searchResults[index];
		event(global::Event::TypeName)= searchResults[index]->getResourceTypeName();
		event.send();
		
		//print(debug::Ch::Dev, debug::Vb::Trivial, "Selected: %s", e.getText(sel.front()).cStr());
	});
	
}

ResourceListEcUi::~ResourceListEcUi(){
	
}

void ResourceListEcUi::update(){
	if (searchGuiUpdateNeeded)
		updateSearchGui();
}

void ResourceListEcUi::onResize(){
	BaseType::onResize();
	
	util::Coord content_rad= getContentElement().getRadius();
	
	searchField.setMaxRadius(content_rad.onlyX() + searchField.getMaxRadius().onlyY());
	
	contentLayout.setMaxRadius(content_rad);
	contentLayout.setMinRadius(content_rad);
	contentLayout.setRadius(content_rad);
	
	util::Coord dif= searchResultsList.getRadius() + searchField.getRadius() - content_rad;
	if ( util::abs(dif.y) > 0.001 || util::abs(dif.x) > 0.001 ){
		util::Coord list_rad = content_rad;
		list_rad.x -= 0.02;
		list_rad.y -= searchField.getRadius().converted(list_rad.getType()).y;
		if (list_rad.y < 0) list_rad.y= 0;
		
		searchResultsList.setMaxRadius(list_rad);
		searchResultsList.setMinRadius(list_rad);
		searchResultsList.setRadius(list_rad);
		searchResultsList.setOffset(searchField.getRadius().onlyY());
		
		maxResultCount= list_rad.y/searchResultsList.getNodeRadius().converted(list_rad.getType()).y*0.8;
		updateSearch(searchField.getText());
	}
}

void ResourceListEcUi::updateSearch(const util::Str8& keyword_str){
	
	searchResults= getComponent().search(keyword_str, maxResultCount);
	
	resourceChangeListener.clear();
	for (const auto& res : searchResults){
		resourceChangeListener.listen(*res, [&] () {
			// Don't call updateSearchGui directly, because dozens of resources 
			// can change at the same time when saving a resource file...
			searchGuiUpdateNeeded= true;
		});
	}

	updateSearchGui();
	
}

void ResourceListEcUi::updateSearchGui(){

	// Memorize selected to be able to select them again when list recreated
	auto selected_array= searchResultsList.getSelected();
	util::DynArray<util::Str8> selected_texts;
	for (const auto& m : selected_array){
		selected_texts.pushBack(searchResultsList.getText(m));
	}
	
	searchResultsList.clear();
	
	for (const auto& res : searchResults){

		gui::ListBoxElement::NodeState state= gui::ListBoxElement::NodeState::Enabled;
		
		if (res->isResourceObsolete())
			state= gui::ListBoxElement::NodeState::Disabled;
		else if (res->getResourceState() == resources::Resource::State::Error)
			state= gui::ListBoxElement::NodeState::Error;
		
		searchResultsList.append("(" + res->getResourceTypeName() + ") " + res->getIdentifierAsString(), gui::ListBoxElement::NodeType::Text, state);
	}
	
	searchResultsList.scrollToBegin();
	
	for (const auto& text : selected_texts){
		if (searchResultsList.hasNode(text)){
			searchResultsList.setSelected(text);
		}
	}
	
	searchGuiUpdateNeeded= false;
}

}}} // ui::game::editor
} // clover