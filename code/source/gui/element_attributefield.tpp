
template <typename T>
AttributeFieldWrapElement<T>::AttributeFieldWrapElement(const resources::AttributeDef& def)
		: attributeDefImpl(&def.getImpl<T>())
		, label(def.getKey(), radius.onlyY())
		, value(AttributeDefImplType::initValue()){
	addSubElement(label);
}

template <typename T>
AttributeFieldWrapElement<T>::~AttributeFieldWrapElement(){
}

template <typename T>
void AttributeFieldWrapElement<T>::addSubElement(Element& e){
	BaseType::addSubElement(e);
	bringTop(label);
}

template <typename T>
void AttributeFieldWrapElement<T>::setLabel(const util::Str8& str){
	label.setText(str);
}

template <typename T>
util::Str8 AttributeFieldWrapElement<T>::getLabel() const {
	return label.getText();
}

template <typename T>
void AttributeFieldWrapElement<T>::setValue(const Value& v){
	value= v;
	onValueSet();
}

template <typename T>
void AttributeFieldWrapElement<T>::setValue(const resources::BaseAttribute& attrib){
	value= attrib.get<T>();
	onValueSet();
}

template <typename T>
std::shared_ptr<resources::BaseAttribute> AttributeFieldWrapElement<T>::getValueAsAttribute() const {
	auto ret= new resources::Attribute<T>(value);
	return std::shared_ptr<resources::BaseAttribute>(ret);
}

template <typename T>
void AttributeFieldWrapElement<T>::onResize(){
	label.setOffset(radius.onlyY());
	/*
	
	print(debug::Ch::Gui, debug::Vb::Trivial, "pasd");
	uint32 i=0;
	for (auto& m : resizeElements){
		print(debug::Ch::Gui, debug::Vb::Trivial, "asd %i", i);
		m->setRadius(BaseType::getRadius());
		m->setMaxRadius(BaseType::getMaxRadius());
		++i;
	}*/
}
