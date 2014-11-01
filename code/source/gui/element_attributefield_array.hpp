#ifndef CLOVER_GUI_ATTRIBUTEFIELD_ARRAY_HPP
#define CLOVER_GUI_ATTRIBUTEFIELD_ARRAY_HPP

#include "build.hpp"
#include "element_attributefield.hpp"
#include "element_button.hpp"
#include "element_layout_linear.hpp"
#include "util/linkedlist.hpp"

namespace clover {
namespace gui {

template <resources::PlainAttributeType PlainType>
class AttributeFieldElement<resources::AttributeType<PlainType, true>> : public AttributeFieldWrapElement<resources::AttributeType<PlainType, true>> {
public:
	using AttributeType= resources::AttributeType<PlainType, true>;
	using Base= AttributeFieldWrapElement<AttributeType>;
	using ArrayElement= AttributeFieldElement<resources::AttributeType<PlainType, false>>;
	using ArrayElementPtr= std::unique_ptr<ArrayElement>;
	
	AttributeFieldElement(const resources::AttributeDef& def)
			: Base(def)
			, layout(LinearLayoutElement::Vertical, util::Coord::P(0), util::Coord::VSt(1))
			, buttonLayout(LinearLayoutElement::Horizontal, util::Coord::P(0), util::Coord::P({100,15}))
			, addButton("Add", util::Coord::P(0), util::Coord::P(10))
			, removeButton("Remove", util::Coord::P(0), util::Coord::P(10)){
		
		Base::addSubElement(layout);
		
		buttonLayout.setMinRadius(buttonLayout.getRadius());
		
		layout.addNode(buttonLayout, 10000);
		buttonLayout.addNode(addButton);
		buttonLayout.addNode(removeButton);
		
		addButton.setOnTriggerCallback([=] (gui::Element&){
			addElement();
		});
		
		removeButton.setOnTriggerCallback([=] (gui::Element&){
			if (!arrayElements.empty()){
				removeElement();
			}
		});
		
	}
	virtual ~AttributeFieldElement(){}

private:
	void addElement(){
		bool value_modified= false;
		if (Base::value.size() == arrayElements.size()){
			Base::value.pushBack(typename ArrayElement::Value()); // Modify value when pressing add-button
			value_modified= true;
		}
		
		ensure(Base::attributeDefImpl);
		arrayElements.pushBack(ArrayElementPtr(new ArrayElement(Base::attributeDefImpl->elementDef)));
		layout.addNode(*arrayElements.back());
		arrayElements.back()->setOnValueModifyCallback([=] (BaseAttributeFieldElement& e){
			callOnValueModify();
		});
		layout.minimizeRadius();
		Base::setRadius(layout.getRadius());
		Base::onResize();
		
		if (value_modified)
			callOnValueModify();
	}

	void removeElement(){
		ensure(!arrayElements.empty());
		
		bool value_modified= false;
		if (Base::value.size() == arrayElements.size()){
			Base::value.popBack(); // Modify value when pressing remove-button
			value_modified= true;
		}
		
		arrayElements.popBack();
		if (!arrayElements.empty())
			layout.spatialUpdate();
		layout.minimizeRadius();
		Base::setRadius(layout.getRadius());
		Base::onResize();
		
		if (value_modified)
			callOnValueModify();
	}

	void callOnValueModify(){
		ensure(Base::value.size() == arrayElements.size());
		for (SizeType i= 0; i < Base::value.size(); ++i){
			Base::value[i]= arrayElements[i]->getValue(); // Update value from elements
		}
		
		Base::callOnValueModify();
	}

	virtual void onValueSet() override {
		while (Base::value.size() > arrayElements.size())
			addElement();
		while (Base::value.size() < arrayElements.size())
			removeElement();
		
		ensure(Base::value.size() == arrayElements.size());
		for (SizeType i= 0; i < Base::value.size(); ++i){
			arrayElements[i]->setValue(Base::value[i]);
		}
	}
	
	LinearLayoutElement layout;
	
	util::DynArray<ArrayElementPtr> arrayElements;
	
	LinearLayoutElement buttonLayout;
	ButtonElement addButton, removeButton;
};

} // gui
} // clover

#endif // CLOVER_GUI_ATTRIBUTEFIELD_ARRAY_HPP