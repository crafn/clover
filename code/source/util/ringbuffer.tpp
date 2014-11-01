template <typename T>
RingBuffer<T>::RingBuffer(SizeType size)
	: buffer(size)
	, readIndex(0)
	, writeIndex(0)
	, usedCount(0){
}

template <typename T>
RingBuffer<T>::RingBuffer(const RingBuffer& other)
	: buffer(other.buffer)
	, readIndex(other.readIndex.load())
	, writeIndex(other.writeIndex.load())
	, usedCount(other.usedCount.load()){
}

template <typename T>
void RingBuffer<T>::resize(SizeType s){
	buffer.resize(s);
}

template <typename T>
SizeType RingBuffer<T>::size() const {
	return buffer.size();
}

template <typename T>
SizeType RingBuffer<T>::getFreeCount() const {
	return buffer.size() - getUsedCount();
}

template <typename T>
bool RingBuffer<T>::isEmpty() const {
	if (getUsedCount() == 0) return true;
	return false;
}

template <typename T>
SizeType RingBuffer<T>::getUsedCount() const {
	return usedCount;
}

template <typename T>
bool RingBuffer<T>::isFull() const {
	if (getUsedCount() == buffer.size()) return true;
	return false;
}

template <typename T>
void RingBuffer<T>::write(const util::DynArray<T>& data) {
	write(data, 0, data.size());
}

template <typename T>
void RingBuffer<T>::write(const util::DynArray<T>& data, SizeType begin, SizeType end) {
	
	debug_ensure(end >= begin);
	debug_ensure(end - begin <= getFreeCount());
	
	for (SizeType i=begin; i<end; ++i){
	
		debug_ensure(writeIndex < buffer.size());
		
		
		
		buffer[writeIndex]= data[i];
		
		if (writeIndex + 1 == buffer.size()) writeIndex= 0;
		else ++writeIndex;
		
		
		++usedCount;
	}
	
}

template <typename T>
util::DynArray<T> RingBuffer<T>::read(SizeType read_count) {
	util::DynArray<T> ret(read_count);
	read(ret, read_count);
	return (ret);
}

template <typename T>
void RingBuffer<T>::read(util::DynArray<T>& target, SizeType read_count) {

	debug_ensure(read_count <= getUsedCount());
	debug_ensure(target.size() >= read_count);
	
	for (SizeType i=0; i<read_count; ++i){
		debug_ensure(readIndex < buffer.size());
	
		target[i]= buffer[readIndex];
		
		
		ensure(usedCount);
		--usedCount;
		
		if (readIndex + 1 == buffer.size()) readIndex= 0;
		else ++readIndex;

	}
}