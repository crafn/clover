template <SignalType S>
SignalValueUi<S>::SignalValueUi():
	viewElement(std::move(SignalValueUiTraits<S>::createViewElement())),
	editElement(std::move(SignalValueUiTraits<S>::createEditElement())),
	bgPanel(util::Coord::VF(0), editElement.getRadius() + util::Coord::VF(0.05)){
	
	floatingElement.addSubElement(bgPanel);
	floatingElement.setApplySuperActivation(false);
	floatingElement.setActive(false);
	
	floatingElement.setOnOutTriggerCallback([&] (gui::FloatingElement& e){
		e.setActive(false);
		
		if (hardware::gMouse->isPressed(GLFW_MOUSE_BUTTON_MIDDLE) || hardware::gMouse->isPressed(GLFW_MOUSE_BUTTON_RIGHT) || hardware::gKeyboard->isDown(GLFW_KEY_DELETE)){
			SignalValueUiTraits<S>::clearView(viewElement);
			onValueUnset();
		}
		else {

			SignalValueUiTraits<S>::valueToView( std::move(SignalValueUiTraits<S>::valueFromEdit(editElement)), viewElement);
			onValueModify();
		}
	});
	
	bgPanel.addSubElement(editElement);

}

template <SignalType S>
void SignalValueUi<S>::showEditControls(bool b){
	floatingElement.setOffset(viewElement.getPosition());
	floatingElement.setActive(b);
	SignalValueUiTraits<S>::onEditActivationChange(editElement);
}

template <SignalType S>
void SignalValueUi<S>::setValue(const util::Any& v, bool only_edit){
	Value value= util::anyCast<typename SignalValueUiTraits<S>::Value>(v);

	SignalValueUiTraits<S>::valueToEdit(value, editElement);
	if (!only_edit)
		SignalValueUiTraits<S>::valueToView(value, viewElement);
}