#include "mesh_impl.hpp"
#include "resources/exception.hpp"

namespace clover {
namespace visual {

#define INIT_TRIMESH_RESOURCE_ATTRIBUTES() \
	INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", ""), \
	INIT_RESOURCE_ATTRIBUTE(verticesAttribute, "vertices", defaultVertices()), \
	INIT_RESOURCE_ATTRIBUTE(indicesAttribute, "indices", defaultIndices())
	
TriMesh::TriMesh(uint32 reserved_vert_count, uint32 reserved_ind_count):
	uniformUvSet(false),
	INIT_TRIMESH_RESOURCE_ATTRIBUTES()
	{

	initAttributeCallbacks();

	vertices.reserve(reserved_vert_count);
	indices.reserve(reserved_ind_count);
	
}

// Dunno if attribute stuff is needed in these,
// because nothing is ever assigned to resources I suppose

TriMesh::TriMesh(const BaseClass& b):
	BaseClass(b),
	uniformUvSet(false),
	INIT_TRIMESH_RESOURCE_ATTRIBUTES(){
	initAttributeCallbacks();
}

TriMesh::TriMesh(const BaseClass::GenMeshBaseClass& b):
		BaseClass(b),
		uniformUvSet(false),
		INIT_TRIMESH_RESOURCE_ATTRIBUTES(){
	initAttributeCallbacks();
}

TriMesh::TriMesh(const TriMesh& other):
		BaseClass(other),
		uniformUvSet(other.uniformUvSet),
		uv(other.uv),
		nameAttribute(other.nameAttribute),
		verticesAttribute(other.verticesAttribute),
		indicesAttribute(other.indicesAttribute){
	initAttributeCallbacks();
}

TriMesh::TriMesh(TriMesh&& other):
		BaseClass(std::move(other)),
		uniformUvSet(std::move(other.uniformUvSet)),
		uv(std::move(other.uv)),
		nameAttribute(std::move(other.nameAttribute)),
		verticesAttribute(std::move(other.verticesAttribute)),
		indicesAttribute(std::move(other.indicesAttribute)){
	initAttributeCallbacks();
}

TriMesh::~TriMesh(){
}

TriMesh& TriMesh::operator=(const TriMesh& other){
	BaseClass::operator=(other);
	uniformUvSet= other.uniformUvSet;
	uv= other.uv;
	
	nameAttribute= other.nameAttribute;
	verticesAttribute= other.verticesAttribute;
	indicesAttribute= other.indicesAttribute;
	
	initAttributeCallbacks();
	
	return *this;
}

void TriMesh::calculateTangents(){
	if (indices.size()%3 != 0) throw(global::Exception("REMesh::calculateTangents(..): index count % 3 != 0"));
	if (vertices.size() == 0 && indices.size() != 0) throw(global::Exception("REMesh::calculateTangents(..): index count == %i but vertex count == %i", indices.size(), vertices.size()));

	for (int32 i=0; i+2<(int)indices.size(); i+=3){
		
		debug_ensure_msg(indices[i] < vertices.size(), "%i, %i", indices[i], vertices.size());
		debug_ensure_msg(indices[i+1] < vertices.size(), "%i, %i", indices[i+1], vertices.size());
		debug_ensure_msg(indices[i+2] < vertices.size(), "%i, %i", indices[i+2], vertices.size());
		
		Vertex* tri[3];

		tri[0]= &vertices[indices[i]];
		tri[1]= &vertices[indices[i+1]];
		tri[2]= &vertices[indices[i+2]];

		calculateTangentTriangle(tri);
	}
	
	setDirty();
}

void TriMesh::add(util::Vec2f pos, util::Vec2d uv){
	Vertex vert;

	vert.position[0]= pos.x;
	vert.position[1]= pos.y;
	vert.uv[0]= uv.x;
	vert.uv[1]= uv.y;

	addVertex(vert);

}

void TriMesh::add(util::Vec2f pos, util::Color color){
	Vertex vert;

	vert.position[0]= pos.x;
	vert.position[1]= pos.y;
	for (int32 i=0; i<4; i++){
		vert.color[i]= color[i];

	}

	addVertex(vert);
}

void TriMesh::add(const TriMesh& mesh){
	for (int32 i=0; i<(int)mesh.indices.size(); i++){
		addIndex(mesh.indices[i]+vertices.size());
	}

	for (int32 i=0; i<(int)mesh.vertices.size(); i++){
		addVertex(mesh.vertices[i]);
	}
}

void TriMesh::addTriangles(const util::DynArray<util::Vec2d>& v){
	ensure(v.size() % 3 == 0);
	uint32 i=0;
	Vertex d[3];
	for (auto &m : v){
		d[i%3].position[0]= m.x; d[i%3].position[1]= m.y;
		++i;

		if (i%3 == 0){

			addTriangle(d);
		}

	}
}

void TriMesh::addTriangles(const util::DynArray<util::Vec2f>& v){
	ensure(v.size() % 3 == 0);
	uint32 i=0;
	Vertex d[3];
	for (auto &m : v){
		d[i%3].position[0]= m.x; d[i%3].position[1]= m.y;

		++i;

		if (i%3 == 0){
			addTriangle(d);
		}
	}
}

void TriMesh::addTriangle(util::Vec2f p1, util::Vec2f p2, util::Vec2f p3){
	Vertex d[3];

	d[0].position[0]= p1.x; d[0].position[1]= p1.y;
	d[1].position[0]= p2.x; d[1].position[1]= p2.y;
	d[2].position[0]= p3.x; d[2].position[1]= p3.y;


	addTriangle(d);
}

void TriMesh::addTriangle(	const util::Vec2f& p1, const util::Vec2f& p2, const util::Vec2f& p3, 
							const util::Color& c1, const util::Color& c2, const util::Color& c3){
	Vertex d[3];

	d[0].position[0]= p1.x; d[0].position[1]= p1.y;
	d[0].color[0]= c1.r; d[0].color[1]= c1.g; d[0].color[2]= c1.b; d[0].color[3]= c1.a;
	d[1].position[0]= p2.x; d[1].position[1]= p2.y;
	d[1].color[0]= c2.r; d[1].color[1]= c2.g; d[1].color[2]= c2.b; d[1].color[3]= c2.a;
	d[2].position[0]= p3.x; d[2].position[1]= p3.y;
	d[2].color[0]= c3.r; d[2].color[1]= c3.g; d[2].color[2]= c3.b; d[2].color[3]= c3.a;
	

	addTriangle(d);		
}

void  TriMesh::add(int32 vertcount, Vertex* d){
	for (int32 i=0; i<vertcount; i++){
		addVertex(d[i]);
	}
}

void TriMesh::addTriangle(Vertex p[3]){
	for(int32 i=0; i<3; i++){
		addVertex(p[i]);
		addIndex(vertices.size()-1);
	}

	return;

	// Don't resolve duplicates automatically

	/*
	for (int32 i=0; i<3; i++){

		bool duplicate= false;

		for (int32 v=0; v<(int)vertices.size(); v++){

			if (p[i].position[0] == vertices[v].position[0] &&
				p[i].position[1] == vertices[v].position[1] &&
				p[i].uv0[0] == vertices[v].uv0[0]           &&
				p[i].uv0[1] == vertices[v].uv0[1]){

				///Duplikaatti, lisätään vain indeksi
				duplicate= true;

				indices.pushBack((uint16)v);


				if (indices.back() > 30000) throw global::Exception("EXCEPTION");

				break;
			}
		}

		if (!duplicate){
			vertices.pushBack(p[i]);
			indices.pushBack(vertices.size()-1);


		}
	}*/

}

void TriMesh::setIndices(const util::DynArray<IndexType>& ind){
	indices= ind;
	setDirty();
}

void TriMesh::overwrite(int32 index, TriMesh& mesh){

	for (uint32 i=0; i<mesh.getVertexCount(); i++){
		vertices[index+i]= mesh.getVertex(i);
	}

	setDirty();
}

void TriMesh::addRectByCenter(const util::Vec2f& pos, const util::Vec2f& rad, const util::Vec2f& uvpos, const util::Vec2f& uvrad){
	if (vertices.empty()){
		uniformUvSet= true;

		util::Vec2f size= uvrad.inversed()*rad;

		util::Vec2f origo= pos - uvpos*size;

		uv.setSize(size);
		uv.setOrigo(origo);
		uv.setRotation(0);
	}
	
	addRectByCorners(pos-rad, pos+rad, uvpos-uvrad, uvpos+uvrad);
}

void TriMesh::addRectByCorners(const util::Vec2f& lower_left, const util::Vec2f& upper_right, const util::Vec2f& uv_lower_left, const util::Vec2f& uv_upper_right){
	if (vertices.empty()){
		uniformUvSet= true;

		util::Vec2f uvrad= (uv_upper_right - uv_lower_left)*0.5;
		util::Vec2f uvpos= (uv_lower_left + uv_upper_right)*0.5;
		util::Vec2f pos= (lower_left + upper_right)*0.5;
		util::Vec2f rad= (upper_right - lower_left)*0.5;

		util::Vec2f size= uvrad.inversed()*rad;

		util::Vec2f origo= pos - uvpos*size;

		uv.setSize(size);
		uv.setOrigo(origo);
		uv.setRotation(0);
	}

	Vertex vert[4];
	vert[0].position[0]= lower_left.x;
	vert[0].position[1]= lower_left.y;
	vert[0].uv[0]= uv_lower_left.x;
	vert[0].uv[1]= uv_lower_left.y;

	vert[1].position[0]= upper_right.x;
	vert[1].position[1]= lower_left.y;
	vert[1].uv[0]= uv_upper_right.x;
	vert[1].uv[1]= uv_lower_left.y;

	vert[2].position[0]= upper_right.x;
	vert[2].position[1]=upper_right.y;
	vert[2].uv[0]= uv_upper_right.x;
	vert[2].uv[1]= uv_upper_right.y;

	vert[3].position[0]= lower_left.x;
	vert[3].position[1]= upper_right.y;
	vert[3].uv[0]= uv_lower_left.x;
	vert[3].uv[1]= uv_upper_right.y;

	IndexType start_index= vertices.size();

	for (int i=0; i<4; ++i)
		addVertex(vert[i]);


	BaseClass::addTriangle(start_index, start_index+1, start_index+2);
	BaseClass::addTriangle(start_index, start_index+2, start_index+3);
}

void TriMesh::setParamByHeight(int32 param, real32 zero_pos, real32 one_pos){
	for (int32 i=0; i<(int)vertices.size(); i++){
		real32 p= (vertices[i].position[1]-zero_pos)/(one_pos-zero_pos);

		vertices[i].params[param]= 1-util::limited(p, 0.0f, 1.0f);
	}
}

void TriMesh::applyUniformUv(const util::UniformUv& uv_){
	uniformUvSet= true;
	uv= uv_;
	for (auto &m : vertices){
		util::Vec2f coord= uv.getUv(util::Vec2f{m.position[0], m.position[1]});
		m.uv[0]= coord.x;
		m.uv[1]= coord.y;
	}
	
	setDirty();
}

void TriMesh::applyUv(const TriMesh& other){
	for (auto& v : vertices){
		
		// Find the triangle in which a vertex is
		for (SizeType i= 0; i+2 < other.indices.size(); i += 3){
			std::array<Vertex, 3> other_v= {
				other.vertices[other.indices[i]],
				other.vertices[other.indices[i+1]],
				other.vertices[other.indices[i+2]]
			};
			
			if (util::geom::isPointInsideTriangle(v.position, other_v[0].position, other_v[1].position, other_v[2].position)){
				// Calculate uv
				util::Vec3d bary= util::geom::barycentric(v.position, other_v[0].position, other_v[1].position, other_v[2].position);
				v.uv= (other_v[0].uv*bary.x + other_v[1].uv*bary.y + other_v[2].uv*bary.z).casted<util::Vec2f>();
				
				break;
			}
		}
		
	}

	setDirty();
}

void TriMesh::translate(util::Vec2f t){
	for (int32 i=0; i<(int32)vertices.size(); i++){
		vertices[i].position[0] += t.x;
		vertices[i].position[1] += t.y;
	}
	
	setDirty();
}

void TriMesh::rotate(real32 rot){
	for (int32 i=0; i<(int32)vertices.size(); i++){
		real32 help= cos(rot)*vertices[i].position[0] - sin(rot)*vertices[i].position[1];
		vertices[i].position[1] = sin(rot)*vertices[i].position[0] + cos(rot)*vertices[i].position[1];
		vertices[i].position[0]= help;
	}
	
	setDirty();
}

void TriMesh::scale(util::Vec2f s){
	for (int32 i=0; i<(int)vertices.size(); i++){
		vertices[i].position[0] *= s.x;
		vertices[i].position[1] *= s.y;
	}
	
	setDirty();
}

void TriMesh::setColor(const util::Color& c, int32 index){
	if(index == -1){
		for (int32 i=0; i<(int)vertices.size(); i++){
			for (int32 a=0; a<4; a++)
				vertices[i].color[a]= c[a];

		}
	}
	else {

		for (int32 a=0; a<4; a++)
			vertices[index].color[a]= c[a];
	}
}

void TriMesh::clear(){
	BaseClass::clear();
	uniformUvSet= false;
}

void TriMesh::resourceUpdate(bool load, bool force){
	// Always loaded
	if (load || getResourceState() == State::Uninit){

		clear();

		vertices= verticesAttribute.get();
		
		for (int64 ind : indicesAttribute.get()){
			indices.pushBack((IndexType)ind);
		}
		
		setDirty();
		
		calculateTangents();

		// Test
		ensure(isSubMesh());

		setResourceState(State::Loaded);
	}
	else {
		setResourceState(State::Unloaded);
	}
}

void TriMesh::createErrorResource(){
	clear();
	addRectByCenter(util::Vec2f(0), util::Vec2f(1.0));
	flush();
}

void TriMesh::initAttributeCallbacks(){
	auto reflush = [&] () {
		clear();
		setResourceState(State::Unloaded);
	};
	
	verticesAttribute.setOnChangeCallback([=] (){
		if (getResourceState() != State::Uninit)
			reflush();
	});
	
	indicesAttribute.setOnChangeCallback([=] (){
		if (getResourceState() != State::Uninit)
			reflush();
	});
}

void TriMesh::calculateTangentTriangle(Vertex* tri[3]){
		util::Vec2d uv[3]= { {tri[0]->uv[0], tri[0]->uv[1]}, {tri[1]->uv[0], tri[1]->uv[1]}, {tri[2]->uv[0], tri[2]->uv[1]} };

		util::Vec2d tangent, bitangent;

		util::Vec2d edge1= {tri[1]->position[0] - tri[0]->position[0], tri[1]->position[1] - tri[0]->position[1]};
		util::Vec2d edge2= {tri[2]->position[0] - tri[0]->position[0], tri[2]->position[1] - tri[0]->position[1]};

		util::Vec2d edge1uv = uv[1] - uv[0];
		util::Vec2d edge2uv = uv[2] - uv[0];

		const real32 cp = edge1uv.y * edge2uv.x - edge1uv.x * edge2uv.y;

		if (cp != 0.0) {
			const real32 mul = 1.0 / cp;

			tangent= (edge1 * -edge2uv.y + edge2 * edge1uv.y) * mul;
			bitangent= (edge1 * -edge2uv.x + edge2 * edge1uv.x) * mul;
			tangent= tangent.normalized();
			bitangent= bitangent.normalized();


			tri[0]->tangent[0] = tangent.x; tri[0]->tangent[1] = tangent.y;

			tri[1]->tangent[0] = tangent.x; tri[1]->tangent[1] = tangent.y;
			tri[2]->tangent[0] = tangent.x; tri[2]->tangent[1] = tangent.y;

		}
}

util::DynArray<Vertex> TriMesh::defaultVertices(){
	util::DynArray<Vertex> v(4);
	v[0].position= util::Vec2f{1.0f, 1.0f};
	v[1].position= util::Vec2f{-1.0f, 1.0f};
	v[2].position= util::Vec2f{-1.0f, -1.0f};
	v[3].position= util::Vec2f{1.0f, -1.0f};
	
	v[0].uv= util::Vec2f{1.0f, 1.0f};
	v[1].uv= util::Vec2f{0.0f, 1.0f};
	v[2].uv= util::Vec2f{0.0f, 0.0f};
	v[3].uv= util::Vec2f{1.0f, 0.0f};
	
	return (v);
}

auto TriMesh::defaultIndices() -> util::DynArray<IndexType> {
	return util::DynArray<IndexType>{0,1,2, 0,2,3};
}

} // visual
} // clover