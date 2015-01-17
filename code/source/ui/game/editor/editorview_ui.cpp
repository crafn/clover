#include "editorview_ui.hpp"
#include "editorcomponent_ui.hpp"
#include "editorcomponent_ui_factory.hpp"
#include "game/editor/editorcomponentfactory.hpp"
#include "gui/gui_mgr.hpp"

namespace clover {
namespace ui { namespace game { namespace editor {

EditorViewUi::EditorViewUi(EditorView& view_):
	view(&view_),
	active(false),
	verticalLayout(gui::LinearLayoutElement::Vertical, util::Coord::VSt(0), util::Coord::VSt({1.0,1.0})){

	auto component_names= EditorComponentFactory::getComponentNames();
		
	// Create component layouts
	for (int32 i=0; i<2; ++i){
		util::Coord rad= util::Coord::VSt({1.0,0.5});
		horizontalLayouts.pushBack(gui::LinearLayoutElement(gui::LinearLayoutElement::Horizontal, util::Coord::VF(0), rad));
		horizontalLayouts.back().setMinRadius(util::Coord::VF(0));
		horizontalLayouts.back().setMargin(false);
		horizontalLayouts.back().setMinSpacing(util::Coord::VF(0.0));
		horizontalLayouts.back().setMaxSpacing(util::Coord::VF(0.0));
		verticalLayout.addSubElement(horizontalLayouts.back());
		
		addCombos.pushBack(std::move(gui::ComboBoxElement(util::Coord::VF(0), util::Coord::VF({addButtonHorizontalRad, addButtonVerticalRad}))));
		addCombos.back().setListNodeVerticalRadius(util::Coord::VF({0, 0.035}));
		
		addCombos.back().setMinRadius(addButtonRad());
		addCombos.back().setMaxRadius(addButtonRad());
		addCombos.back().setRadius(addButtonRad());
		addCombos.back().setManualRadius();
		
		horizontalLayouts.back().addNode(addCombos.back(), 999); /// 999 to stay on right side
		
		for (auto& m : component_names)
			addCombos.back().append(m);
		
		addCombos.back().setOnSelectCallback([=](gui::Element& e){
			gui::ComboBoxElement& combo= *static_cast<gui::ComboBoxElement*>(&e);
			
			ensure(combo.getSelectedIndex() >= 0);

			createComponent(combo.getSelectedText(), i);
			
			combo.resetListPosition();
		});
		
	}
	
	verticalLayout.setActive(false);
	verticalLayout.setMargin(false);
	
	listenForEvent(global::Event::EditorComponentUiDestroyRequest);
	listenForEvent(global::Event::OnEditorComponentUiUserResize);
	listenForEvent(global::Event::OnEditorResourceSelect);

	print(debug::Ch::Resources, debug::Vb::Trivial, "EditorViewUi created");
}

EditorViewUi::~EditorViewUi(){

	print(debug::Ch::Resources, debug::Vb::Trivial, "EditorViewUi destroyed");
}

void EditorViewUi::onEvent(global::Event& e){
	if (!active) return;
	
	switch(e.getType()){

		case global::Event::EditorComponentUiDestroyRequest:
			destroyComponent(*e(global::Event::Object).getPtr<BaseEditorComponentUi>());
		break;
		
		case global::Event::OnEditorComponentUiUserResize:
			limitComponents();
			adjustLayouts();
		break;
		
		case global::Event::OnEditorResourceSelect:{
			if (!active)
				break;
			
			resources::Resource* res= e(global::Event::Resource).getPtr<resources::Resource>();
			util::Str8 res_type_name= e(global::Event::TypeName).getStr();
			
			util::Str8 comp_type_name= EditorComponentFactory::getCorrespondingResourceComponentName(res_type_name);
			
			// Create component if there isn't one
			
			if (view->hasComponent(comp_type_name))
				break;
			
			int row= 0; // Upper row is default
			
			// Change to next row if current has no room (= less than half of default width of the component)
			while (row < 2 && 
				calcRowEmptySpace(horizontalLayouts[row]).converted(util::Coord::View_Stretch).getValue().x
				< RuntimeEditorComponentUiTraits::defaultRadius(comp_type_name).converted(util::Coord::View_Stretch).getValue().x){
				++row;
			}
			
			if (row > 1)
				break; // All rows full

			EditorComponent& comp= createComponent(comp_type_name, row).getComponent();
			
			// Forward event because component wasn't listening at the moment when event was sent
			comp.onEvent(e);
		}
		break;

		default: break;
	}

}

void EditorViewUi::update(){
	if (!active) return;
	
	// Probably not necessary to call if components haven't changed
	adjustLayouts();
	
	for (auto& m : componentUis){
		m->update();
	}
}

void EditorViewUi::setActive(bool b){
	active= b;
	verticalLayout.setActive(b);
}

void EditorViewUi::toggle(){
	setActive(!active);
}

BaseEditorComponentUi& EditorViewUi::createComponent(const util::Str8& name, int32 row){
			
	EditorComponent& comp= view->createComponent(name);
	componentUis.pushBack(EditorComponentUiFactory::create(comp));
	ensure(componentUis.back()->getSuperGuiElement().isActive());
	horizontalLayouts[row].addSubElement(componentUis.back()->getSuperGuiElement());
	ensure(componentUis.back()->getSuperGuiElement().isActive());

	limitComponents();
	adjustLayouts();
	
	return *componentUis.back();
}

void EditorViewUi::destroyComponent(BaseEditorComponentUi& comp_ui){

	for (auto it = componentUis.begin(); it != componentUis.end(); ++it){
		if (&comp_ui == it->get()){
			// Destroy everything
			view->destroyComponent(comp_ui.getComponent());
			componentUis.erase(it);
			
			limitComponents();
			adjustLayouts();
			
			return;
		}
	}
	
	ensure_msg(0, "Component UI not found");
}

util::Coord EditorViewUi::calcRowEmptySpace(const gui::LinearLayoutElement& horizontal, bool use_min_radiuses){
	util::Coord total(util::Coord::View_Stretch);
	total.setRelative();
	
	for (auto& element : horizontal.getSubElements()){
		
		if (use_min_radiuses)
			total += util::Coord::VSt(0.1); // Kind of hacky
		else if (element->getType() == gui::Element::Panel) // Don't count combo buttons
			total += element->getRadius().converted(util::Coord::View_Stretch)*2;
	}
	
	
	return util::Coord::VSt(2) - total;
}

void EditorViewUi::adjustElementY(gui::Element& e, int32 row, const util::Coord& min_height){
	if (row == 0){
		// Top
		e.setOffset(e.getOffset().onlyX() + min_height.onlyY() - e.getRadius().onlyY());
	}
	else {
		// Bottom
		e.setOffset(e.getOffset().onlyX() - min_height.onlyY() +e.getRadius().onlyY());
	}
}

util::Coord EditorViewUi::calcSumOfLayoutVerticalMin() const {
	util::Coord sum_of_layout_vertical_min(0, util::Coord::View_Stretch);
	sum_of_layout_vertical_min.setRelative();
	
	for (auto& layout : horizontalLayouts){
		if (layout.getSubElements().size() > 0){
			
			bool count_size= false;
			if (layout.getSubElements().size() == 1){
				if (layout.getSubElements().front()->getType() == gui::Element::Panel){
					count_size= true;
				}
			}
			else {
				count_size= true;
			}

			if (count_size)
				sum_of_layout_vertical_min += layout.getMinRadius();

		}
	}
	
	return sum_of_layout_vertical_min;
}

void EditorViewUi::limitComponents(){

	util::Coord sum_of_layout_vertical_min= calcSumOfLayoutVerticalMin();
	
	// Limit resize radius
	for (uint32 row=0; row < horizontalLayouts.size(); ++row){
		auto& layout= horizontalLayouts[row];
		
		// X
		util::Coord free_space= calcRowEmptySpace(layout);
		
		const auto& subelements= layout.getSubElements();
		
		for (int i= subelements.size() - 1; i >= 0; --i) {
			auto& element= subelements[i];
			
			for (auto it = componentUis.begin(); it != componentUis.end(); ++it){
				if (element == &it->get()->getSuperGuiElement()){
					util::Coord max(util::Coord::View_Stretch);
					
					max.x= (element->getRadius()*2 + free_space).converted(util::Coord::View_Stretch).x*0.5;
					
					max.y= 1 - (sum_of_layout_vertical_min - layout.getMinRadius()).converted(util::Coord::View_Stretch).y;
					
					if (max.y < 0) max.y= 0;
					if (max.x < 0) max.x= 0;
					
					util::Coord old_rad= element->getRadius();
					
					it->get()->setMaxRadius(max);
					
					// Only the last one is made smaller if components are using too much space
					// If the last one can't be made small enough, second last is also resized etc.
					if (free_space.x < 0){
						free_space= free_space + old_rad*2 - it->get()->getMaxRadius()*2;
					}
					
					break;
				}
			}
			
		}
		
	}
}

void EditorViewUi::adjustLayouts(){
	util::Coord sum_of_layout_vertical_min= calcSumOfLayoutVerticalMin();
	
	for (uint32 row=0; row < horizontalLayouts.size(); ++row){
		
		// Ajust height
		util::Coord min_height(-1, util::Coord::View_Stretch); // Minimum height for layout
		for (auto& m : horizontalLayouts[row].getSubElements()){
			if (min_height.x < 0 || m->getRadius().converted(util::Coord::View_Stretch).y > min_height.y){
				min_height %= m->getRadius();
			}
		}
		
		if (min_height.y < 0)
			min_height.y= 0;
			
		if (!horizontalLayouts[row].getSubElements().empty()){
			horizontalLayouts[row].setMaxRadius(util::Coord::VSt({1, min_height.y}));
			horizontalLayouts[row].setRadius(util::Coord::VSt({1, min_height.y}));
			horizontalLayouts[row].setMinRadius(util::Coord::VSt({1, min_height.y}));
		}
		
		// Adjust components next to the edge of the window
		for (auto& m : horizontalLayouts[row].getSubElements()){
			adjustElementY(*m, row, min_height);
		}
		
		real64 free_horizontal_space= calcRowEmptySpace(horizontalLayouts[row]).converted(util::Coord::View_Fit).x;
		
		real64 free_vertical_space= util::Coord::VSt(2).converted(util::Coord::View_Fit).y 
									- sum_of_layout_vertical_min.converted(util::Coord::View_Fit).y*2;
		
		
		bool enough_horizontal_space= free_horizontal_space*0.5 > addButtonHorizontalRad;
		bool enough_vertical_space= free_vertical_space*0.5 > addButtonVerticalRad;
		
		if (free_horizontal_space*0.5 < util::Coord::VSt(1).converted(util::Coord::View_Fit).x)
			enough_vertical_space= true; // Always room vertically if there's other components in row
		
		// Hide add-button
		if (addCombos[row].isActive() && (!enough_horizontal_space || !enough_vertical_space)){
			addCombos[row].setActive(false);
			
			horizontalLayouts[row].removeSubElement(addCombos[row]);
			if (!enough_vertical_space){
				horizontalLayouts[row].setMaxRadius(util::Coord::VF({1, 0}));
				horizontalLayouts[row].setRadius(util::Coord::VF({1, 0}));
				horizontalLayouts[row].setMinRadius(util::Coord::VF({1, 0}));
			}
			
			continue;
		}

		if (free_horizontal_space*0.5 < util::Coord::VSt(1).converted(util::Coord::View_Fit).x)
			enough_vertical_space= true; 
		
		// Show add-button
		if (!addCombos[row].isActive() && enough_vertical_space && enough_horizontal_space){
			addCombos[row].setActive(true);
			
			horizontalLayouts[row].addNode(addCombos[row], 999);
			
			if (horizontalLayouts[row].getMaxRadius().y <= util::epsilon){
				// Adding button to an empty layout
				addCombos[row].setOffset(addCombos[row].getOffset().onlyX());
				horizontalLayouts[row].setMaxRadius(util::Coord::VF({0, addButtonVerticalRad}) + util::Coord::VSt(1).onlyX());
				horizontalLayouts[row].setRadius(util::Coord::VF({0, addButtonVerticalRad}) + util::Coord::VSt(1).onlyX());
				horizontalLayouts[row].setMinRadius(util::Coord::VSt({1, 0}));
			}
			else {
				adjustElementY(addCombos[row], row, min_height);
				horizontalLayouts[row].setMaxRadius(horizontalLayouts[row].getRadius().onlyY() + util::Coord::VSt(1).onlyX());
				horizontalLayouts[row].setRadius(horizontalLayouts[row].getRadius().onlyY() + util::Coord::VSt(1).onlyX());
				horizontalLayouts[row].setMinRadius(horizontalLayouts[row].getRadius().onlyY() + util::Coord::VSt(1).onlyX());
			}
			
			
			//print(debug::Ch::Dev, debug::Vb::Trivial, "Show");
		}
		if (row == 0){
			//print(debug::Ch::Dev, debug::Vb::Trivial, "Free space: %f, %f, Row %i", free_horizontal_space, free_vertical_space, row);
		}
	}
}

}}} // ui::game::editor
} // clover
