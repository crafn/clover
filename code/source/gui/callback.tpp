template <typename T>
Callback<T>::Callback():
	owner(0){
}

template <typename T>
void Callback<T>::assign(const CallbackFunction<T>& f){
	callback= f;
}

template <typename T>
void Callback<T>::trigger(ElementType& element){
	owner= &element;
	
	if (!callback) return;
	
	// Maybe direct call would be sufficient..
	global::Event e(global::Event::OnGuiCallbackTrigger);
	e(global::Event::Object)= this;
	e.send();
}

template <typename T>
void Callback<T>::call(){
	ensure(owner);
	ensure(callback);
	
	callback(*owner);
}