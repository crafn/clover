template <typename V, typename I>
GpuMesh<V,I>::GpuMesh():
		BaseMesh(GenMeshBaseClass::getContentHash()),
		vao(0),
		superMesh(nullptr),
		subVertexOffset(0),
		subIndexOffset(0){
}

template <typename V, typename I>
GpuMesh<V,I>::GpuMesh(const GpuMesh& other):
	BaseMesh(GenMeshBaseClass::getContentHash()),
	GenMeshBaseClass(other),
	vao(0),
	superMesh(nullptr),
	subVertexOffset(0),
	subIndexOffset(0){
}

template <typename V, typename I>
GpuMesh<V,I>::GpuMesh(const GenMeshBaseClass& b):
	BaseMesh(GenMeshBaseClass::getContentHash()),
	GenMeshBaseClass(b),
	vao(0),
	superMesh(nullptr),
	subVertexOffset(0),
	subIndexOffset(0){
}

template <typename V, typename I>
GpuMesh<V,I>::GpuMesh(GpuMesh&& other):
	BaseMesh(GenMeshBaseClass::getContentHash()),
	GenMeshBaseClass(std::move(other)),
	vao(0),
	superMesh(nullptr),
	subVertexOffset(0),
	subIndexOffset(0){
	if (!other.superMesh && other.vao){
		vao= other.vao;
		other.vao= 0;
	}
	ensure_msg(!other.superMesh, "Not implemented");
}

template <typename V, typename I>
GpuMesh<V,I>::~GpuMesh(){
	if (!superMesh && vao)
		destroyVao();
}

template <typename V, typename I>
GpuMesh<V,I>& GpuMesh<V,I>::operator=(GpuMesh&& other){
	GenMeshBaseClass::operator=(std::move(other));
	vao= 0;
	superMesh= nullptr;
	subVertexOffset= 0;
	subIndexOffset= 0;
	if (!other.superMesh && other.vao){
		vao= other.vao;
		other.vao= 0;
	}
	ensure_msg(!superMesh && !other.superMesh, "Not implemented");
	return *this;
}


template <typename V, typename I>
GpuMesh<V,I>& GpuMesh<V,I>::operator=(const GpuMesh& other){
	GenMeshBaseClass::operator=(other);
	ensure_msg(!superMesh, "Not implemented"); /// @todo Handle submesh case
	superMesh= nullptr;
	subVertexOffset= 0;
	subIndexOffset= 0;

	return *this;
}

template <typename V, typename I>
void GpuMesh<V,I>::addSubMesh(GpuMesh& m){
	subMeshInfos.pushBack(SubMeshInfo{&m, 0, 0});
	m.superMesh= this;
}

template <typename V, typename I>
void GpuMesh<V,I>::setUsage(hardware::GlState::VaoUsage u){
	ensure(!superMesh);
	if (!vao)
		createVao();

	vao->setUsage(u);
}

template <typename V, typename I>
void GpuMesh<V,I>::onSubMeshFlush(VaoType* external_vao, uint32 vertex_offset, uint32 index_offset){
	ensure(vao == 0 || vao == external_vao);
	ensure(superMesh);
	vao= external_vao;
	subVertexOffset= vertex_offset;
	subIndexOffset= index_offset;
}

template <typename V, typename I>
void GpuMesh<V,I>::clear(){
	GenMeshBaseClass::clear();

	for (auto& info : subMeshInfos)
		info.mesh->clear();
}

template <typename V, typename I>
void GpuMesh<V,I>::flush() const {

	if (superMesh){
	
		const SubMeshInfo& info= superMesh->getSubMeshInfo(*this);
		
		if (vao && info.vertexCount == vertices.size() && info.indexCount == indices.size()){
			// Overwrite external vao
			//print(debug::Ch::General,debug::Vb::Trivial, "subV: %i, subI: %i, %i %i", subVertexOffset, subIndexOffset, vertices.size(), indices.size());
			vao->overwrite(asArrayView(vertices), subVertexOffset);

			util::DynArray<IndexType> offset_indices= indices;

			for (auto& m : offset_indices){
				m += subVertexOffset;
			}
			if (vao->isIndexed())
				vao->overwrite(asArrayView(offset_indices), subIndexOffset);
		}
		else {
			// Super mesh has not created buffer yet or own size has changed
			superMesh->flush();
		}
	}
	else {

		uint32 total_vertex_count= vertices.size();
		uint32 total_index_count= indices.size();

		for (const auto& info : subMeshInfos){
			total_vertex_count += info.mesh->getVertexCount();
			total_index_count += info.mesh->getIndexCount();
		}

		// Own VAO
		if (!vao){
			createVao();
		}

		ensure(vao);

		vao->reserveVertices(total_vertex_count);
		if (vao->isIndexed())
			vao->reserveIndices(total_index_count);

		if (total_vertex_count == 0 || ( total_index_count == 0 && vao->isIndexed() ))
			return;

		uint32 vertex_offset= 0;
		uint32 index_offset= 0;

		vao->overwrite(asArrayView(vertices));
		if (vao->isIndexed())
			vao->overwrite(asArrayView(indices));

		vertex_offset += vertices.size();
		index_offset += indices.size();

		for (auto& info : subMeshInfos){
			info.mesh->onSubMeshFlush(vao, vertex_offset, index_offset);
			
			info.vertexCount= info.mesh->getVertexCount();
			info.indexCount= info.mesh->getIndexCount();
			
			vertex_offset += info.vertexCount;
			index_offset += info.indexCount;

			info.mesh->flush();
		}

	}
}

template <typename V, typename I>
void GpuMesh<V,I>::draw() const {
	if (isDirty())
		fixDirtyness();
		
	if (!vao)
		return;
	
	if (superMesh){
		vao->drawRange(subIndexOffset, subIndexOffset + indices.size());
	}
	else {
		vao->draw();

		for (const auto& info : subMeshInfos){
			info.mesh->draw();
		}
	}
}

template <typename V, typename I>
void GpuMesh<V,I>::createVao() const {
	createVaoImpl(hardware::GlState::Primitive::Triangle);
}

template <typename V, typename I>
auto GpuMesh<V,I>::getBoundingBox() const -> BoundingBox {
	return GenMeshBaseClass::getBoundingBox().template casted<BoundingBox>();
}

template <typename V, typename I>
void GpuMesh<V,I>::createVaoImpl(hardware::GlState::Primitive p) const {
	ensure(!vao);
	vao= new VaoType(p);
}

template <typename V, typename I>
void GpuMesh<V,I>::fixDirtyness() const {
	flush();

	GenMeshBaseClass::fixDirtyness();
}

template <typename V, typename I>
void GpuMesh<V,I>::destroyVao() const {
	ensure(vao);
	delete vao;
	vao= 0;
}

template <typename V, typename I>
const typename GpuMesh<V,I>::SubMeshInfo& GpuMesh<V,I>::getSubMeshInfo(const GpuMesh& m) const {

	const SubMeshInfo* found= nullptr;
	for (const auto& info : subMeshInfos){
		if (info.mesh == &m){
			found= &info;
			break;
		}
	}
	
	ensure(found);
	return *found;
}

template <typename V>
PointMesh<V>::PointMesh():
	drawOffset(0), drawCount(0){

}

template <typename V>
PointMesh<V>::~PointMesh(){

}

template <typename V>
void PointMesh<V>::setDrawRange(uint32 offset, uint32 count)
{
	drawOffset= offset;
	drawCount= count;
}

template <typename V>
void PointMesh<V>::draw() const {
	const typename BaseClass::VaoType* vao= BaseClass::getVao();

	if (!vao || drawCount == 0) return;

	ensure(!vao->isIndexed());
	ensure(vao->getPrimitive() == hardware::GlState::Primitive::Point);
	ensure(drawCount <= vao->getVertexCount());
	ensure(drawOffset < vao->getVertexCount());

	uint32 second_draw= 0;
	if (drawOffset + drawCount >= vao->getVertexCount()){
		second_draw= drawOffset + drawCount - vao->getVertexCount();
	}

	/// @todo Support for submeshing
	ensure(!BaseClass::isSubMesh());

	vao->drawRange(drawOffset, drawOffset + drawCount - second_draw);

	if (second_draw > 0)
		vao->drawRange(0, second_draw);

}

template <typename V>
void PointMesh<V>::createVao() const {
	BaseClass::createVaoImpl(hardware::GlState::Primitive::Point);
}
