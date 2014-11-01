template <typename T>
SizeType RawArchive::serializePod(uint8* buf, const T* ptr, SizeType count){
	SizeType byte_count= sizeof(T)*count;
	memcpy(buf, ptr, byte_count);
	return byte_count;
}

template <typename T>
SizeType RawArchive::deserializePod(const uint8* buf, T* ptr, SizeType count){
	SizeType byte_count= sizeof(T)*count;
	memcpy(ptr, buf, byte_count);
	return byte_count;
}