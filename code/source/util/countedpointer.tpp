template <typename T>
CountedPointer<T>::CountedPointer(T* v){
	ptr= v;
	if (ptr)
		incr();
}

template <typename T>
CountedPointer<T>::CountedPointer(const CountedPointer& other){
	ptr= other.ptr;
	if (ptr)
		incr();
}

template <typename T>
CountedPointer<T>::CountedPointer(CountedPointer&& other){
	ptr= other.ptr;
	other.ptr= 0;
}

template <typename T>
CountedPointer<T>::~CountedPointer(){
	if (ptr)
		decr();
}

template <typename T>
CountedPointer<T>& CountedPointer<T>::operator=(T* v){
	if (ptr != v){
		if (ptr)
			decr();
		
		ptr= v;
		
		if (ptr)
			incr();
	}
	return *this;
}

template <typename T>
CountedPointer<T>& CountedPointer<T>::operator=(const CountedPointer& other){
	if (&other != this){
		if (ptr)
			decr();

		ptr= other.ptr;
		
		if (ptr)
			incr();
	}
	
	return *this;
}

template <typename T>
CountedPointer<T>& CountedPointer<T>::operator=(CountedPointer&& other){
	if (&other != this){
		if (ptr)
			decr();
			
		ptr= other.ptr;
		
		if (ptr)
			incr();
	}
	
	return *this;
}

template <typename T>
bool CountedPointer<T>::operator==(const CountedPointer& other) const {
	return ptr == other.ptr;
}

template <typename T>
bool CountedPointer<T>::operator==(T* v) const {
	return ptr == v;
}

template <typename T>
CountedPointer<T>::operator bool() const {
	return ptr != nullptr;
}

template <typename T>
T* CountedPointer<T>::operator->() {
	debug_ensure(ptr);
	return ptr;
}

template <typename T>
const T* CountedPointer<T>::operator->() const {
	debug_ensure(ptr);
	
	return ptr;
}

template <typename T>
T& CountedPointer<T>::operator*(){
	debug_ensure(ptr);
	return *ptr;
}

template <typename T>
const T& CountedPointer<T>::operator*() const {
	debug_ensure(ptr);
	return *ptr;
}

template <typename T>
T* CountedPointer<T>::get(){
	return ptr;
}

template <typename T>
const T* CountedPointer<T>::get() const {
	return ptr;
}

template <typename T>
void CountedPointer<T>::incr(){
	static_cast<const ReferenceCountable*>(ptr)->incrementReferenceCount();
}

template <typename T>
void CountedPointer<T>::decr(){
	static_cast<const ReferenceCountable*>(ptr)->decrementReferenceCount();
}