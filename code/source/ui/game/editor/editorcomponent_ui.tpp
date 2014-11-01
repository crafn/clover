template<typename T>
EditorComponentUi<T>::EditorComponentUi(T& comp):
	radius(EditorComponentUiTraits<ComponentType>::defaultRadius()),
	component(&comp),
	bgPanel(util::Coord::VF(0), radius),
	panelLayout(gui::LinearLayoutElement::Vertical, util::Coord::VF(0), util::Coord::VF(0)),
	titleLabel(EditorComponentTraits<ComponentType>::name(), util::Coord::VF(0)),
	resizeButtonLayout(gui::LinearLayoutElement::Horizontal, util::Coord::VF(0), util::Coord::VF(0)),
	resizeButton(util::Coord::VF(0), util::Coord::VF({resizeButtonVerticalRad*5, resizeButtonVerticalRad})),
	resizing(false),
	contentPanel(util::Coord::VF(0), util::Coord::VF(0)){
	
	bgPanel.addSubElement(panelLayout);
	
	panelLayout.setMargin(false);

	panelLayout.setMaxSpacing(util::Coord::VF(0.1));
	panelLayout.setMinSpacing(util::Coord::VF(0));
	
	panelLayout.addSubElement(titleLabel);
	panelLayout.addSubElement(contentPanel);
	contentPanel.setStretchable();
	panelLayout.addSubElement(resizeButtonLayout);
	
	resizeButtonLayout.setMargin(false);
	resizeButtonLayout.mirrorFirstNodeSide();
	resizeButtonLayout.addSubElement(resizeButton);
	
	setMaxRadius(util::Coord::VSt(1.0));
	setRadius(radius);
	
	
	resizeButton.setOnDraggingStartCallback([&] (gui::Element& caller){
		resizing= true;
		resizeRadius= radius;
		
		//gUserInput->pushCursorLock();
	});
	
	resizeButton.setOnDraggingCallback([&] (gui::Element& caller){
		
		// Had to add "this->" so gcc 4.7.2 wouldn't segfault :p (internal compiler error: Segmentation fault)
		
		resizeRadius += gUserInput->getCursorDifference().converted(util::Coord::View_Stretch)*util::Vec2d{0.5,-0.5};
		if (resizeRadius.x < 0) resizeRadius.x= 0;
		if (resizeRadius.y < 0) resizeRadius.y= 0;
		this->setRadius(this->resizeRadius);
		
		global::Event e(global::Event::OnEditorComponentUiUserResize);
		e(global::Event::Object)= this;
		e.send();
	});
	
	resizeButton.setOnDraggingStopCallback([&] (gui::Element& caller){
		resizing= false;
		//gUserInput->popCursorLock();
	});
	
	titleLabel.setTouchable();
	titleLabel.setOnTriggerCallback([&](gui::Element& caller){
		global::Event e(global::Event::EditorComponentUiDestroyRequest);
		e(global::Event::Object)= this;
		e.send();
	});
	
	onResize();
}

template<typename T>
EditorComponentUi<T>::~EditorComponentUi(){

}

template<typename T>
util::Str8 EditorComponentUi<T>::getComponentName(){
	return EditorComponentTraits<ComponentType>::name();
}

template<typename T>
gui::Element& EditorComponentUi<T>::getSuperGuiElement(){
	return bgPanel;
}

template<typename T>
void EditorComponentUi<T>::setAdditionalTitle(const util::Str8& t){
	titleLabel.setText(EditorComponentTraits<ComponentType>::name() + " - " + t);
}

template<typename T>
T& EditorComponentUi<T>::getComponent(){
	return *component;
}

template<typename T>
void EditorComponentUi<T>::setMaxRadius(const util::Coord& max_radius_){
	util::Coord max_radius= max_radius_.converted(radius.getType());
	if (max_radius.y < 0.1) max_radius.y= 0.1;
	if (max_radius.x < 0.1) max_radius.x= 0.1;
	
	bool changed= false;
	
	if (util::abs(maxRadius.x - max_radius.x) > util::epsilon ||
		util::abs(maxRadius.y - max_radius.y) > util::epsilon)
		changed= true;
	
	maxRadius= max_radius.converted(util::Coord::View_Stretch);
	setRadius(getRadius()); // Limit
	
	if (changed)
		onResize();
}

template<typename T>
void EditorComponentUi<T>::setRadius(const util::Coord& rad){
	
	util::Coord rad_v= rad.converted(util::Coord::View_Stretch);
	util::Coord max_v= maxRadius.converted(util::Coord::View_Stretch);
	
	
	// Limit radius
	if (rad_v.y < 0.1) rad_v.y= 0.1;
	else if (rad_v.y > max_v.y) rad_v.y= max_v.y;
	
	if (rad_v.x < 0.1) rad_v.x= 0.1;
	else if (rad_v.x > max_v.x) rad_v.x= max_v.x;
	
	bool changed= false;
	
	if (util::abs(radius.x - rad.converted(radius.getType()).x) > util::epsilon ||
		util::abs(radius.y - rad.converted(radius.getType()).y) > util::epsilon)
		changed= true;
	
	radius= rad_v.converted(radius.getType());
	
	if (changed)
		onResize();
}

template<typename T>
const util::Coord&  EditorComponentUi<T>::getRadius() const {
	return radius;
}

template<typename T>
void EditorComponentUi<T>::onResize(){
	
	bgPanel.setRadius(radius);
	bgPanel.setMaxRadius(radius);
	bgPanel.setMinRadius(radius);
	
	panelLayout.setMaxRadius(radius);
	panelLayout.setMinRadius(radius);
	panelLayout.setRadius(radius);
	
	util::Coord resize_layout_rad= util::Coord::VF({0,resizeButton.getRadius().y}) + util::Coord({radius.x, 0}, radius.getType());
	resizeButtonLayout.setMaxRadius(resize_layout_rad);
	resizeButtonLayout.setMinRadius(resize_layout_rad);
	resizeButtonLayout.setRadius(resize_layout_rad);
	
	util::Coord content_rad= radius - util::Coord::VF({0,resizeButtonVerticalRad + titleLabel.getRadius().converted(util::Coord::View_Fit).y});
	if (content_rad.x < 0) content_rad.x= 0;
	if (content_rad.y < 0) content_rad.y= 0;
	contentPanel.setMaxRadius(content_rad);
	contentPanel.setMinRadius(content_rad);
	contentPanel.setRadius(content_rad);
	
}