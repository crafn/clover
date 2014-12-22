template <typename V, typename I>
GenericMesh<V, I>::GenericMesh(){
}

template <typename V, typename I>
template <typename VV, typename II>
GenericMesh<VV, II> GenericMesh<V, I>::converted() const {
	GenericMesh<VV, II> c;

	for (uint32 i=0; i<vertices.size(); ++i)
		c.addVertex(MeshVertexTraits<VV>::converted(vertices[i]));

	for (uint32 i=0; i<indices.size(); ++i)
		c.addIndex((II) indices[i]);

	return (c);
}

template <typename V, typename I>
template <class M>
M GenericMesh<V, I>::converted() const {
	return M(converted<typename M::VType, typename M::IType>());
}

template <typename V, typename I>
void GenericMesh<V, I>::addVertex(const V& v){
	vertices.pushBack(v);
	contentHash += util::hash32(v);
	dirty= true;
}

template <typename V, typename I>
void GenericMesh<V, I>::addTriangle(const V& a, const V& b, const V& c){
	I start_index= vertices.size();
	addVertex(a);
	addVertex(b);
	addVertex(c);
	addIndex(start_index);
	addIndex(start_index+1);
	addIndex(start_index+2);

	dirty= true;
}

template <typename V, typename I>
const V& GenericMesh<V, I>::getVertex(const I& i) const {
	debug_ensure(i < vertices.size());
	return vertices[i];
}

template <typename V, typename I>
void GenericMesh<V, I>::setVertex(const I& i, const V& v) {
	debug_ensure(i < vertices.size());
	vertices[i]= v;
	/// @todo Recalculate contentHash
	dirty= true;
}

template <typename V, typename I>
void GenericMesh<V, I>::setVertices(util::DynArray<V> v){
	vertices= std::move(v);
	/// @todo Recalculate contentHash
	dirty= true;
}

template <typename V, typename I>
void GenericMesh<V, I>::addIndex(const IType& i){
	indices.pushBack(i);
	contentHash += util::hash32(i);
	dirty= true;
}

template <typename V, typename I>
void GenericMesh<V, I>::addTriangle(const I& a, const I& b, const I& c){
	addIndex(a);
	addIndex(b);
	addIndex(c);
}

template <typename V, typename I>
const I& GenericMesh<V, I>::getIndex(uint32 i) const {
	debug_ensure(i < indices.size());
	return indices[i];
}

template <typename V, typename I>
void GenericMesh<V, I>::setIndex(uint32 i, const I& ind) {
	debug_ensure(i < indices.size());
	indices[i]= ind;

	dirty= true;
}

template <typename V, typename I>
void GenericMesh<V, I>::setIndices(util::DynArray<I> i) {
	indices= std::move(i);
	/// @todo Recalculate contentHash
	dirty= true;
}

template <typename V, typename I>
auto GenericMesh<V, I>::getBoundingBox() const -> util::BoundingBox<BType> {
	if (dirty)
		fixDirtyness();

	return boundingBox;
}

template <typename V, typename I>
void GenericMesh<V, I>::setBoundingBox(const util::BoundingBox<BType>& bb){
	if (isDirty())
		fixDirtyness();

	boundingBox= bb;
}

template <typename V, typename I>
void GenericMesh<V, I>::fixDirtyness() const {
	ensure(dirty);
	boundingBox.reset();
	
	for (const auto& m : vertices){
		boundingBox.append(MeshVertexTraits<V>::extractPosition(m));
	}
	
	dirty= false;
}

template <typename V, typename I>
void GenericMesh<V, I>::clear(){
	vertices.clear();
	indices.clear();
	dirty= true;
	contentHash= 0;
}

template <typename V, typename I>
const uint32& GenericMesh<V, I>::getContentHash() const {
	return contentHash;
}
