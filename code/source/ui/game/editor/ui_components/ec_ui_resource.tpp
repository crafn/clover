// BaseResourceEcUi

template <typename T>
BaseResourceEcUi<T>::BaseResourceEcUi(T& comp, bool show_attributes)
	: EditorComponentUi<T>(comp)
	, createDialogLayout(gui::LinearLayoutElement::Vertical)
	, createDialogIdentifierAttributeDef(resources::ResourceTraits<ResourceType>::getAttributeDefs().front())
	, createDialogIdentifier(createDialogIdentifierAttributeDef){
	
	Base::getComponent().addOnResourceSelectCallback([&] (){
		if (Base::getComponent().getSelectedResource())
			onResourceSelect(*Base::getComponent().getSelectedResource());
		else
			onResourceChange();
	});
	
	attributeDefs= resources::ResourceTraits<ResourceType>::getAttributeDefs();
	attributeDefs.popFront(); // Don't create identifier field
	
	Base::getContentElement().setOnSecondaryTriggerCallback([&] (gui::Element&) {
		contextMenu.show(gUserInput->getCursorPosition());
	});
	
	createResourceNodeId= contextMenu.append("Create new resource");
	destroyResourceNodeId= contextMenu.append("Destroy this resource");
	
	contextMenu.setOnSelectCallback([&] (gui::ContextMenuElement&){
		auto selected_id= contextMenu.getSelected().front();
		
		if (selected_id == createResourceNodeId){
			createDialogFloating.show(gUserInput->getCursorPosition());
		}
		else if (selected_id == destroyResourceNodeId){
			auto res= Base::getComponent().getSelectedResource();
			if (res){
				Base::getComponent().clearSelectedResource();
				res->setResourceObsolete();
			}
		}
		
	});
	
	// create -dialog
	createDialogFloating.setActive(false);
	createDialogFloating.addSubElement(createDialogBgPanel);
	
	util::Coord rad= util::Coord::P({250,50});
	createDialogBgPanel.setRadius(rad);
	createDialogBgPanel.setMaxRadius(rad);
	createDialogBgPanel.addSubElement(createDialogLayout);
	
	createDialogLayout.setRadius(rad);
	createDialogLayout.setMaxRadius(rad);
	createDialogLayout.addSubElement(createDialogResFileCombo);
	createDialogLayout.addSubElement(createDialogIdentifier);
	createDialogLayout.addSubElement(createDialogCreateButton);
	createDialogCreateButton.setText("Create");
	
	for (const auto& res_file : Base::getComponent().getResourceFilePaths()){
		createDialogResFileCombo.append(res_file.fromRoot());
	}
	
	createDialogFloating.setOnOutTriggerCallback([&] (gui::FloatingElement&){
		if (!gUserInput->isTriggered(UserInput::GuiConfirm)) // Explicit press on button is required to confirm, or click outside to close
			createDialogFloating.setActive(false);
	});
	
	createDialogCreateButton.setOnTriggerCallback([&] (gui::Element&){
		// Couldn't just write logic here, because gcc 4.8.0 kept complaining about redeclaration of "this"
		onCreateButtonPress();
	});
	
	print(debug::Ch::Dev, debug::Vb::Trivial, "ResourceEcUi<%s> created", EditorComponentUi<T>::getComponentName().cStr());
}

template <typename T>
BaseResourceEcUi<T>::~BaseResourceEcUi(){
	print(debug::Ch::Dev, debug::Vb::Trivial, "ResourceEcUi<%s> destroyed", EditorComponentUi<T>::getComponentName().cStr());
}

template <typename T>
void BaseResourceEcUi<T>::onResourceSelect(ResourceType& res){

	resourceChangeListener.clear();
	resourceChangeListener.listen(res, [&] () {
		// Update gui when resource changes
		/// @todo Prevent calling when modified in editor
		onResourceChange();
	});
	
	print(debug::Ch::Dev, debug::Vb::Trivial, "Event OnEditorResourceSelect received: %s", getSelectedResource()->getResourceTypeName().cStr());
	
	onResourceChange();
	
}

template <typename T>
void BaseResourceEcUi<T>::onResourceChange(){
		
	if (!getSelectedResource() || getSelectedResource()->isResourceObsolete()){
		clear();
		return;
	}
	
	Base::getSuperGuiElement().setErrorVisuals(getSelectedResource()->getResourceState() == resources::Resource::State::Error);
	
	Base::setAdditionalTitle(getSelectedResource()->getIdentifierAsString());
	
	if (attributeFields.empty())
		return;
	uint32 i=0;
	for (auto it= attributeDefs.begin(); it != attributeDefs.end(); ++it){
		attributeFields[i].get()->setValue(getSelectedResource()->getResourceAttribute(it->getKey()));
		++i;
	}
	
	
}

template <typename T>
void BaseResourceEcUi<T>::clear(){
	resourceChangeListener.clear();
	Base::setAdditionalTitle("");
	Base::getSuperGuiElement().setErrorVisuals(false);
	clearAttributeFields();
}

template <typename T>
void BaseResourceEcUi<T>::createAttributeFields(gui::Element* super){
	for (auto it= attributeDefs.begin(); it != attributeDefs.end(); ++it){
		auto& def = *it;
		attributeFields.pushBack(FieldPtr(gui::AttributeFieldElementFactory::create(def)));
		
		initAttributeField(attributeFields.back(), def, super);
		
	}
}

template <typename T>
void BaseResourceEcUi<T>::clearAttributeFields(){

	// A better way to do this would be calling clear() for every attributeField, but they don't have the method..
	for (auto it= attributeFields.begin(); it != attributeFields.end(); ++it){
		FieldPtr& field= *it;
		const resources::AttributeDef& def= attributeDefs[it - attributeFields.begin()];
		
		gui::Element* super= field->getSuperElement();
		field= FieldPtr(gui::AttributeFieldElementFactory::create(def));
		
		initAttributeField(field, def, super);
		
	}
}

template <typename T>
void BaseResourceEcUi<T>::initAttributeField(FieldPtr& field, const resources::AttributeDef& def, gui::Element* super){
	
	// Change resource's attribute when fields in gui change
	field->setOnValueModifyCallback([=] (gui::BaseAttributeFieldElement& caller) {
		if (getSelectedResource()){
			getSelectedResource()->getResourceAttribute(caller.getLabel())
				.template set(*caller.getValueAsAttribute());
		}
	});
	
	if (super)
		super->addSubElement(*field);
}

template <typename T>
void BaseResourceEcUi<T>::onCreateButtonPress(){
	try {
		Base::getComponent().selectResource(Base::getComponent().createResource(
			createDialogIdentifier.getValue(),
			Base::getComponent().getResourceFilePaths().at(createDialogResFileCombo.getSelectedIndex())));
		createDialogFloating.setActive(false);
	}
	catch (const resources::ResourceException&){
	}
}


//
// ResourceEcUi
//

template <typename T>
ResourceEcUi<T>::ResourceEcUi(T& comp):
	Base(comp),
	contentLayout(gui::LinearLayoutElement::Vertical, util::Coord::VF(0), Base::getRadius()){
	
	Base::getContentElement().addSubElement(contentLayout);
	
	Base::createAttributeFields(&contentLayout);
}

template <typename T>
ResourceEcUi<T>::~ResourceEcUi(){
}

template <typename T>
void ResourceEcUi<T>::onResize(){
	Base::onResize();
	
	contentLayout.setMaxRadius(Base::getContentElement().getRadius());
	contentLayout.setRadius(Base::getContentElement().getRadius());
	
	/*
	for (auto& m : Base::attributeFields){
		util::Coord r= m->getMaxRadius();
		r.x= Base::getContentElement().getTargetRadius().converted(r.getType()).x;
		m->setMaxRadius(r);
		m->setRadius(r);
		m->onResize();
	}*/
}
