
template <typename T>
Attribute<T>::Attribute(const Value& v):
	value(v),
	addedToCont(false){
}

template <typename T>
Attribute<T>::Attribute(const AttributeInitializer<AttributeType>& init):
	initializer(init),
	value(initializer.getValue()),
	addedToCont(false){
	
	if (initializer.getContainer()){
	
		//print(debug::Ch::Resources, debug::Vb::Trivial, "Adding attribute to container (%i): %s", initializer.getContainer(), initializer.getKey().cStr());
		
		// Overrides other's pointer in container
		auto it= initializer.getContainer()->find(initializer.getKey());
		ensure_msg(it == initializer.getContainer()->end(), "Duplicate attribute: %s, same object: %i", initializer.getKey().cStr(), it->second == this);
		
		//print(debug::Ch::Resources, debug::Vb::Trivial, "Container size: %i", initializer.getContainer()->size());
		
		uint32 size= initializer.getContainer()->size();
		initializer.getContainer()->operator[](initializer.getKey())= this;
		
		ensure(size + 1 == initializer.getContainer()->size());
		
		addedToCont= true;
	}
}

template <typename T>
Attribute<T>::Attribute(const Attribute& other):
	value(other.value),
	OnChange(other.OnChange){
}

template <typename T>
Attribute<T>::Attribute(Attribute&& other):
	value(std::move(other.value)),
	OnChange(std::move(other.OnChange)){
}

template <typename T>
Attribute<T>::~Attribute(){
	if (initializer.getContainer()){
		auto it= initializer.getContainer()->find(initializer.getKey());
		
		//print(debug::Ch::Resources, debug::Vb::Trivial, "Removing attribute from container (%i): %s", initializer.getContainer(), initializer.getKey().cStr());
		
		if (it == initializer.getContainer()->end()){
			for (auto& m : *initializer.getContainer()){
				print(debug::Ch::Resources, debug::Vb::Trivial, "Attribute: %s", m.second->getKey().cStr());
			}
		}
		
		ensure(addedToCont);
		ensure_msg(it != initializer.getContainer()->end(), "Attribute not in container (%i), key: %s, container size: %i",
			initializer.getContainer(), initializer.getKey().cStr(), initializer.getContainer()->size());

		initializer.getContainer()->erase(it);
	}
}

template <typename T>
Attribute<T>& Attribute<T>::operator=(const Attribute& other){
	initializer.setKey(other.initializer.getKey());
	initializer.setValue(other.initializer.getValue());
	value= other.value;
	OnChange= other.OnChange;
	
	return *this;
}

template <typename T>
Attribute<T>& Attribute<T>::operator=(Attribute&& other){
	initializer.setKey(other.initializer.getKey());
	initializer.setValue(other.initializer.getValue());
	value= other.value;
	OnChange= other.OnChange;
	
	return *this;
}

template <typename T>
const util::Str8& Attribute<T>::getKey() const {
	return initializer.getKey();
}

template <typename T>
util::ObjectNode Attribute<T>::serialized() const {
	return AttributeDefImpl<T>::serialized(value);
}

template <typename T>
void Attribute<T>::set(const Value& v){
	value= v;
	
	tryLaunchAttributeOnChangeCallbacks(OnChange, initializer.getOwner());
}

template <typename T>
auto Attribute<T>::get() const -> const Value& {
	return value;
}

template <typename T>
auto Attribute<T>::get() -> Value& {
	return value;
}

template <typename T>
void Attribute<T>::reset(){
	set(initializer.getValue());
}

template <typename T>
void Attribute<T>::setOnChangeCallback(const OnChangeCallback& f){
	OnChange= f;
}