#include "shape.hpp"
#include "collision/baseshape_polygon.hpp"
#include "collision/baseshape_circle.hpp"
#include "global/event.hpp"
#include "util/ensure.hpp"
#include "util/polygon.hpp"

namespace clover {
namespace collision {

Shape::Shape()
		: INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(baseShapesAttribute, "baseShapes", {}){
}

Shape::Shape(const Shape& other)
		: INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(baseShapesAttribute, "baseShapes", {}){
	deepCopy(other);
}

Shape::~Shape(){
}

Shape& Shape::operator=(const Shape& other){
	deepCopy(other);
	return *this;
}

Shape::SubShapeId Shape::add(const BaseShape& shape){
	baseShapesAttribute.get().pushBack(BaseShapePtr(shape.clone()));
	return baseShapesAttribute.get().size()-1;
}

void Shape::addMesh(const MeshType& m){
	if (m.getVertexCount() == 0)
		return;
	
	// Supports only indexed meshes
	ensure(m.getIndexCount() > 0);

	for (uint32 i=0; i<m.getIndexCount(); i+=3){

		util::DynArray<util::Vec2d> tri;
		tri.resize(3);


		for (int32 a=0; a<3; a++){
			util::Vec2d vert= m.getVertex(m.getIndex(i+a));
			tri[a]= vert;
		}

		// Too small triangles aren't allowed
		if (util::geom::polygonArea(tri) <= PolygonBaseShape::minArea) continue;

		collision::PolygonBaseShape poly;
		poly.setVertices(tri);

		add(poly);
	}
}

void Shape::transform(const Transform& t){
	for (auto& shp : baseShapesAttribute.get()){
		shp->transform(t);
	}
}

void Shape::mirror(){
	for (auto& shp : baseShapesAttribute.get()){
		shp->mirror();
	}
}

util::DynArray<util::Polygon> Shape::asPolygons(real64 imprecision) const {
	util::DynArray<util::Polygon> ret;
	for (const auto& shp : baseShapesAttribute.get()){
		ret.pushBack(shp->asPolygon(imprecision));
	}
	return ret;
}

util::DynArray<util::Polygon> Shape::asConvexPolygons(real64 imprecision) const
{
	util::DynArray<util::Polygon> ret;
	for (const auto& shp : baseShapesAttribute.get()) {
		for (const auto& p : shp->asConvexPolygons(imprecision)) {
			ret.pushBack(p);
		}
	}
	return ret;
}

util::DynArray<util::Polygon> Shape::asUnionPolygons(real64 imprecision) const {
	auto polys= asPolygons(imprecision);
	util::PolyClipper clipper;
	clipper.addSubjects(polys);
	return clipper.execute(util::PolyClipper::Union);
}

bool Shape::overlaps(const collision::Ray& ray, const Transform& transform, collision::RayCastResult* output) const {
	for (auto& m : baseShapesAttribute.get()){
		/// @todo Choose closest to the starting point of the ray
		if (m->overlaps(ray, transform, output)) return true;
	}
	return false;
}

bool Shape::overlaps(const util::Vec2d& pos, const Transform& transform) const {
	for (auto& m : baseShapesAttribute.get()){
		if (m->overlaps(pos, transform))
			return true;
	}
	return false;
}


bool Shape::overlaps(	const Shape& shape,
						const Transform& shape_transform,
						const Transform& transform) const {
	for (auto& m : baseShapesAttribute.get()){
		for (auto& other_m : shape.getBaseShapes()){
			if (m->overlaps(*other_m, shape_transform, transform))
				return true;
		}
	}
	return false;
}

real64 Shape::distance(	const Shape& shape,
						const Transform& shape_transform,
						const Transform& transform) const {
	real64 closest_dist= std::numeric_limits<real64>::max();
	for (auto& m : baseShapesAttribute.get()){
		for (auto& other_m : shape.getBaseShapes()){
			real64 dist= m->distance(*other_m, shape_transform, transform);
			closest_dist= std::min(closest_dist, dist);
		}
	}
	return closest_dist;
}

Shape Shape::erasedBy(const Shape& other, const Transform& other_transform, const Transform& this_transform, real64 imprecision, bool boolean_and) const {
	/// @todo Preserve circles
	util::PolyClipper clipper;
	
	auto this_polys= asPolygons(imprecision);
	for (auto& poly : this_polys)
		poly.transform(this_transform);
		
	auto other_polys= other.asPolygons(imprecision);
	for (auto& poly : other_polys)
		poly.transform(other_transform);
	
	clipper.addSubjects(this_polys);
	clipper.addClippers(other_polys);
	
	auto clip_type= util::PolyClipper::Difference;
	if (boolean_and)
		clip_type= util::PolyClipper::Intersection;
	this_polys= clipper.execute(clip_type);
	
	for (auto& poly : this_polys)
		poly.transform(this_transform.inversed());
	
	Shape result_shape;
	for (auto& poly : this_polys){
		if (poly.getVertexCount() < 3)
			continue;
		result_shape.add(PolygonBaseShape(poly.getVertices()));
	}
	return result_shape;
}

void Shape::clean(real64 min_area){
	auto this_polys= asPolygons(sqrt(min_area));
	// Remove too small
	for (auto it= this_polys.begin(); it != this_polys.end();){
		if (it->getVertexCount() < 3 || it->getArea() < min_area){
			SizeType i= it - this_polys.begin();
			baseShapesAttribute.get().erase(
				baseShapesAttribute.get().begin() + i);
			it= this_polys.erase(it);
		}
		else
			++it;
	}
}

void Shape::resourceUpdate(bool load, bool force){
	if (getResourceState() == State::Uninit || load){
		setResourceState(State::Loaded);
	}
	else {
		setResourceState(State::Unloaded);
	}
}

void Shape::createErrorResource(){
	baseShapesAttribute.get().clear();
	
	CircleBaseShape circle(util::Vec2d(0), 0.5);
	add(circle);
	
	setResourceState(State::Error);
}

void Shape::deepCopy(const Shape& other){
	nameAttribute= other.nameAttribute;
	baseShapesAttribute.get().clear();
	for (const auto& shape : other.getBaseShapes())
		add(*shape);
		
	util::OnChangeCb::trigger();
}

} // collision
} // clover
