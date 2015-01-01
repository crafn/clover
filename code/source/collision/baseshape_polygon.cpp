#include "baseshape_polygon.hpp"
#include "box2d.hpp"
#include "util/vector.hpp"
#include "util/dyn_array.hpp"
#include "util/polygon.hpp"

namespace clover {
namespace collision {

PolygonBaseShape::PolygonBaseShape(const util::DynArray<util::Vec2d>& v){
	if (!v.empty())
		setVertices(v);
}

PolygonBaseShape::~PolygonBaseShape(){
}

void PolygonBaseShape::setVertices(util::DynArray<util::Vec2d> v){
	poly.clear();
	poly.append(v);
	if (poly.isClockwise())
		poly.reverse();

	util::DynArray<util::Polygon>
		convex_polys= poly.splittedToConvex(maxInternalVertexCount);
	shapes.clear();

	SizeType i= 0;
	for (util::Polygon& p : convex_polys){
		// 0.5*b2_linearSlop treshold is used in Box2D to remove duplicates
		p.simplify(0.5*b2_linearSlop);

		if (p.getArea() < minArea)
			continue;

		if (p.getVertexCount() < 3)
			continue;

		if (p.isClockwise()){
			//print(debug::Ch::Phys, debug::Vb::Trivial, "PolygonBaseShape::setVertices: clockwise polygon, reversing");
			p.reverse();
		}

		ensure(!p.isClockwise());

		uint32 count= p.getVertexCount();
		ensure(count <= maxInternalVertexCount);
		ensure(count > 0);

		b2Vec2 bvec[count];
		for (uint32 i=0; i<count; i++){
			bvec[i]= toB2(p.getVertex(i));
		}

		shapes.emplaceBack();
		shapes.back().Set(bvec, count);
		++i;
	}

	b2ShapeCount= shapes.size();
}

util::DynArray<util::Vec2d> PolygonBaseShape::getVertices() const {
	return poly.getVertices();
}

void PolygonBaseShape::set(const util::Polygon& p){
	ensure(p.getVertexCount() > 0);
	setVertices(p.getVertices());
}

void PolygonBaseShape::setAsRect(const util::Vec2d& pos, const util::Vec2d& rad, real64 angle){
	util::Polygon p;
	p.appendRect(pos, rad);
	p.rotate(angle);
	set(p);
}

void PolygonBaseShape::transform(const Transform& t){
	util::DynArray<util::Vec2d> p= poly.getVertices();
	for (SizeType i= 0; i < p.size(); ++i){
		p[i]= (Transform(0.0, p[i]) * t).translation;
	}
	setVertices(p);
}

void PolygonBaseShape::mirror(){
	util::DynArray<util::Vec2d> p= poly.getVertices();
	for (SizeType i= 0; i < p.size(); ++i){
		p[i].x *= -1.0;
	}
	setVertices(p);
}

const b2Shape& PolygonBaseShape::getB2Shape(SizeType i) const {
	ensure(i < shapes.size());
	return shapes[i];
}

util::Polygon PolygonBaseShape::asPolygon(real64 imprecision) const {
	return poly;
}

util::DynArray<util::Polygon>
PolygonBaseShape::asConvexPolygons(real64 imprecision) const {
	/// @todo Precision adjusting
	return poly.splittedToConvex();
}

PolygonBaseShape* PolygonBaseShape::clone() const {
	return new PolygonBaseShape(*this);
}

} // collision
} // clover
