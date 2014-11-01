#include "createnodeslotdialog.hpp"
#include "ui/userinput.hpp"

namespace clover {
namespace ui { namespace nodes {

CreateNodeSlotDialog::CreateNodeSlotDialog()
		: layout(gui::LinearLayoutElement::Vertical)
		, nameField(gui::TextFieldElement::Type::Dev){
	floating.setActive(false);
	floating.addSubElement(bgPanel);
	
	util::Coord rad= util::Coord::P({250,50});
	bgPanel.setRadius(rad);
	bgPanel.setMaxRadius(rad);
	bgPanel.addSubElement(layout);
	
	layout.setRadius(rad);
	layout.setMaxRadius(rad);
	layout.addSubElement(typeCombo);
	layout.addSubElement(nameField);
	layout.addSubElement(createButton);
	createButton.setText("Create");
	
	nameField.setCharacterWidth(20);
	nameField.setMaxRadius(nameField.getRadius());
	
	floating.setActive(true);
	
	for (auto signal_type : RuntimeSignalTypeTraits::signalTypes()){
		typeCombo.append(RuntimeSignalTypeTraits::enumString(signal_type));
	}
	
	floating.setOnOutTriggerCallback([&] (gui::FloatingElement&){
		if (!gUserInput->isTriggered(UserInput::GuiConfirm)){
			floating.setActive(false);
			if (onClose)
				onClose(SignalType::None, "");
		}
	});
	
	createButton.setOnTriggerCallback([&] (gui::Element&){
		floating.setActive(false);
		
		if (onClose)
			onClose((SignalType)typeCombo.getSelectedIndex(), nameField.getText());
	});
	
}

void CreateNodeSlotDialog::setOnCloseCallback(Callback cb){
	onClose= cb;
}

}} // ui::nodes
} // clover