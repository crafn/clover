
template <typename V, typename I>
VertexArrayObject<V, I>::VertexArrayObject(hardware::GlState::Primitive p):
	BaseVertexArrayObject(	p,
							sizeof(V),
							hardware::GlState::TypeToEnumType<I>::value()){

	const auto& attributes=  V::getAttributes();

	uint32 id_counter= 0;
	for (const auto& m : attributes){
		BaseVertexArrayObject::setVertexAttribute(id_counter, m.offset, m.elemType, m.elemCount, m.normalized, true);
		//print(debug::Ch::General, debug::Vb::Trivial, "Attrib: %i %i %i %i %i %s", id_counter, m.offset, m.type, m.count, m.normalized, m.name.cStr());
		id_counter += m.arraySize;
	}

}

template <typename V, typename I>
VertexArrayObject<V, I>::~VertexArrayObject(){
}

template <typename V, typename I>
void VertexArrayObject<V, I>::submit(util::ArrayView<const V> vertices){
	/*if (vertices.size() == getVertexCount())
		overwriteData(hardware::GlState::VaoBufferType::Vertex, 0, &vertices[0], vertices.size());
	else*/
		submitData(hardware::GlState::VaoBufferType::Vertex, &vertices[0], vertices.size());
}

template <typename V, typename I>
void VertexArrayObject<V, I>::reserveVertices(uint32 count){
	submitData(hardware::GlState::VaoBufferType::Vertex, 0, count);
}

template <typename V, typename I>
void VertexArrayObject<V, I>::overwrite(util::ArrayView<const V> vertices, uint32 offset){
	overwriteData(hardware::GlState::VaoBufferType::Vertex, offset, &vertices[0], vertices.size());
}

template <typename V, typename I>
void VertexArrayObject<V, I>::submit(util::ArrayView<const I> indices){
	ensure(isIndexed());
	/*if (indices.size() == getIndexCount())
		overwriteData(hardware::GlState::VaoBufferType::Index, 0, &indices[0], indices.size());
	else*/
		submitData(hardware::GlState::VaoBufferType::Index, &indices[0], indices.size());
}

template <typename V, typename I>
void VertexArrayObject<V, I>::reserveIndices(uint32 count){
	ensure(isIndexed());
	submitData(hardware::GlState::VaoBufferType::Index, 0, count);
}

template <typename V, typename I>
void VertexArrayObject<V, I>::overwrite(util::ArrayView<const I> indices, uint32 offset){
	ensure(isIndexed());
	overwriteData(hardware::GlState::VaoBufferType::Index, offset, &indices[0], indices.size());
}