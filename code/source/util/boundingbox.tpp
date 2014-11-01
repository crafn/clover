template <typename T>
BoundingBox<T>::BoundingBox(){
	reset();
}

template <typename T>
BoundingBox<T>::BoundingBox(const T& min_, const T& max_):
	min(min_), max(max_){
}

template <typename T>
BoundingBox<T> BoundingBox<T>::operator+(const T& v) const {

	return (BoundingBox(min + v, max + v));
}

template <typename T>
void BoundingBox<T>::append(const T& pos){
	if (!isSet()){ // Not set
		min= max= pos;
		return;
	}

	for (SizeType i= 0; i < detail::componentCount<T>(); ++i){
		if (detail::component(pos, i) < detail::component(min, i)){
			detail::component(min, i)= detail::component(pos, i);
		}
		if (detail::component(pos, i) > detail::component(max, i)){
			detail::component(max, i)= detail::component(pos, i);
		}
	}
}

template <typename T>
void BoundingBox<T>::append(const This& other){
	if (!isSet()){
		min= other.min;
		max= other.max;
		return;
	}

	append(other.min);
	append(other.max);
}

template <typename T>
void BoundingBox<T>::reset(){
	min= T(0);
	max= T(-1);
}

template <typename T>
bool BoundingBox<T>::isSet() const {
	return detail::component(max, 0) >= detail::component(min, 0);
}
