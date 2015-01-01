#include "baseshape_circle.hpp"
#include "box2d.hpp"
#include "util/math.hpp"

namespace clover {
namespace collision {

util::DynArray<util::Vec2d> circleVertices(util::Vec2d center, real64 radius, int32 vcount){
	util::DynArray<util::Vec2d> verts;
	for (int32 i= 0; i < vcount; ++i){
		real64 angle= util::tau*i/vcount;
		
		util::Vec2d unit= util::Vec2d{cos(angle), sin(angle)};
		verts.pushBack(unit*radius + center);
	}
	return verts;
}

CircleBaseShape::CircleBaseShape(const util::Vec2d& pos, real64 r){
	b2ShapeCount= 1;
	
	shape.m_p= toB2(pos);
	shape.m_radius= r;
}

CircleBaseShape::~CircleBaseShape(){
}

void CircleBaseShape::setPosition(const util::Vec2d& pos)
{ shape.m_p= toB2(pos); }
util::Vec2d CircleBaseShape::getPosition() const
{ return fromB2(shape.m_p); }

void CircleBaseShape::transform(const Transform& t){
	shape.m_p= toB2((Transform(0.0, fromB2(shape.m_p))*t).translation);
}

void CircleBaseShape::mirror(){
	shape.m_p.x *= -1.0;
}

const b2Shape& CircleBaseShape::getB2Shape(SizeType i) const {
	return shape;
}

util::Polygon CircleBaseShape::asPolygon(real64 imprecision) const {
	int32 vcount= util::tau*shape.m_radius/imprecision;
	util::Polygon p;
	for (auto& v : circleVertices(fromB2(shape.m_p), shape.m_radius, vcount)){
		p.append(v);
	}	
	return p;
}

util::DynArray<util::Polygon>
CircleBaseShape::asConvexPolygons(real64 imprecision) const {
	return {asPolygon(imprecision)};
}

CircleBaseShape* CircleBaseShape::clone() const {
	return new CircleBaseShape(*this);
}

} // collision
} // clover
