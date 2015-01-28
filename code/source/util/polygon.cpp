#include "polygon.hpp"
#include "debug/print.hpp"
#include "util/profiling.hpp"

#include <algorithm>
#include <clipper/clipper.hpp>
#include <polypartition.h>

/// For test
#include <iostream>

namespace clover {
namespace util {

struct UnitTest {
	UnitTest(){
		Polygon p;
		p.append(util::Vec2d{0.0, 0.0});
		p.append(util::Vec2d{0.5, 0.5});
		p.append(util::Vec2d{0.5, 0.5});
		p.append(util::Vec2d{0.0, 1.0});

		real64 area= 0.0;

		auto tris= p.splittedToConvex(3);

		for (auto t : tris){
			for (SizeType i= 0; i < t.getVertexCount(); ++i)
				std::cout << "v " << t.getVertex(i).x << ", " << t.getVertex(i).y << std::endl;

			std::cout << "A " << t.getArea() << std::endl;
			std::cout << "isClockwise " << t.isClockwise() << std::endl;
			std::cout << "isConvex" << t.isConvex() << std::endl;
			area += t.getArea();
			ensure(t.isConvex());
			ensure(!t.isClockwise());
		}

		ensure(abs(area - p.getArea()) < 0.01);

		//util::DynArray<Polygon> polys= p.splittedToConvex();
		//ensure(polys.size() == 1);
		//ensure(polys.front().getVertexCount() == 4);
	}
};

//UnitTest u;

ClipperLib::IntPoint Polygon::toFixed(const util::Vec2d& v2) const {
	ClipperLib::IntPoint v1;
	v1.X= v2.x*Polygon::precisionMul;
	v1.Y= v2.y*Polygon::precisionMul;
	return v1;
}
util::Vec2d Polygon::toFloating(const ClipperLib::IntPoint& v2) const {
	util::Vec2d v1;
	v1.x= v2.X/Polygon::precisionMul;
	v1.y= v2.Y/Polygon::precisionMul;
	return v1;
}

ClipperLib::IntPoint operator/(const ClipperLib::IntPoint& v, const real64& t){
	return ClipperLib::IntPoint(v.X/t, v.Y/t);
}

ClipperLib::IntPoint operator*(const ClipperLib::IntPoint& v, const real64& t){
	return ClipperLib::IntPoint(v.X*t, v.Y*t);
}

ClipperLib::IntPoint operator+(const ClipperLib::IntPoint& v1, const ClipperLib::IntPoint& v2){
	return ClipperLib::IntPoint(v1.X + v2.X, v1.Y + v2.Y);
}

ClipperLib::IntPoint operator-(const ClipperLib::IntPoint& v1, const ClipperLib::IntPoint& v2){
	return ClipperLib::IntPoint(v1.X - v2.X, v1.Y - v2.Y);
}

Polygon Polygon::extrudedLineSegment(const util::DynArray<util::Vec2d>& points, real64 extrude_length){
	if (points.empty())
		return Polygon();

	Polygon poly;

	util::Vec2d last_dir;
	for (SizeType i= 0; i + 1 < points.size(); ++i){
		util::Vec2d segment= points[i + 1] - points[i];

		util::Vec2d dir;
		dir= -segment.normal().normalized();

		if (i == 0) // First point
			poly.append(points[i] + dir*extrude_length);
		else
			poly.append(points[i] + (dir + last_dir)*0.5*extrude_length);

		if (i + 2 == points.size()) // Last point
			poly.append(points[i + 1] + dir*extrude_length);

		last_dir= dir;
	}

	// Second side
	for (SizeType i= 0; i < points.size(); ++i){
		SizeType index= points.size() - i - 1;
		ensure(index < poly.getVertices().size() && index < points.size());

		poly.append(points[index]*2.0 - poly.getVertices()[index]);
	}

	return poly;
}

Polygon::Polygon(){
}

void Polygon::append(util::Vec2d point){
	poly.pushBack(point);
}

void Polygon::append(util::DynArray<util::Vec2d> vertices){
	for (auto &m : vertices){
		poly.pushBack(m);
	}
}

void Polygon::popBack(){
	ensure(!poly.empty());
	poly.popBack();
}

void Polygon::erase(SizeType i){
	ensure(i < poly.size());
	poly.erase(poly.begin() + i);
}

void Polygon::appendRect(util::Vec2d center, util::Vec2d rad){
	util::DynArray<util::Vec2d> quad;
	quad.pushBack(center + rad);
	quad.pushBack(center + util::Vec2d{rad.x, -rad.y});
	quad.pushBack(center - rad);
	quad.pushBack(center + util::Vec2d{-rad.x, rad.y});

	append(quad);
}

void Polygon::transform(util::RtTransform2d t){
	for (auto& v : poly){
		v= v*t;
	}
}

void Polygon::translate(util::Vec2d t){
	for (auto& v : poly){
		v= v + t;
	}
}

void Polygon::rotate(real64 angle){
	for (auto& m : poly){
		m= m.rotated(angle);
	}
}

void Polygon::scale(real64 mul){
	for (auto& m : poly)
		m *= mul;
}

void Polygon::scaleAround(util::Vec2d p, real64 mul){
	for (auto& m : poly){
		util::Vec2d scaled= (m - p)*mul + p;
		m= scaled;
	}
}

void Polygon::clear(){
	poly.clear();
}

void Polygon::subdivide(){
	if (poly.size() < 3) return;
	util::DynArray<util::Vec2d> vec;
	for (uint32 i=0; i<poly.size()-1; i++){
		vec.pushBack(poly[i]);
		vec.pushBack((poly[i]+poly[i+1])/2.0);
	}
	vec.pushBack(poly.back());
	vec.pushBack((poly.back()+poly[0])/2.0);

	poly= std::move(vec);
}


void Polygon::addNoise(real64 amount){
	for (auto &m : poly){
		m.x += util::Rand::continuous<real64>(-amount, amount);
		m.y += util::Rand::continuous<real64>(-amount, amount);
	}
}

void Polygon::setVertex(SizeType i, util::Vec2d v){
	ensure(i < getVertexCount());
	poly[i]= v;
}


util::Vec2d Polygon::getVertex(SizeType i) const {
	ensure(i < getVertexCount());
	return poly[i];
}

util::DynArray<util::Vec2d> Polygon::getVertices() const {
	util::DynArray<util::Vec2d> result(poly.size());
	for (uint32 i=0; i<result.size(); ++i){
		result[i]= poly[i];
	}
	return result;
}

util::Vec2d Polygon::getCenter() const {
	ensure(!empty());

	util::Vec2d ret;
	for (SizeType i= 0; i < poly.size(); ++i){
		ret += poly[i];
	}
	return ret/(real64)poly.size();
}

util::Vec2d Polygon::getBoundingRadius() const {
	util::Vec2d center= getCenter();

	util::Vec2d rad;
	for (SizeType i= 0; i < getVertexCount(); ++i){
		util::Vec2d v= getVertex(i) - center;
		if (std::abs(v.x) > rad.x)
			rad.x= std::abs(v.x);
		if (std::abs(v.y) > rad.y)
			rad.y= std::abs(v.y);
	}
	return rad;
}

real64 Polygon::getArea() const {
	real64 area= 0.0;
	for (SizeType i= 0; i < getVertexCount(); ++i) {
		auto first= getVertex(i);
		auto second= getVertex((i + 1) % getVertexCount());
		real64 dx= second.x - first.x;
		area += dx*first.y + dx*second.y;
	}
	return std::abs(area/2.0);
}

real64 Polygon::getDistance(SizeType i, SizeType k) const {
	const SizeType vcount= getVertexCount();
	ensure(i < vcount && k < vcount);

	real64 dist= 0.0;
	real64 other_dist= 0.0;
	for (SizeType count= 0; count < vcount; ++count){
		SizeType cur= (i + count) % vcount;
		SizeType next= (i + 1) % vcount;

		if (cur == k){
			other_dist= dist;
			dist= 0.0;
		}

		dist += (getVertex(cur) - getVertex(next)).length();
	}

	return dist < other_dist ? dist : other_dist;
}

util::GenericMesh<util::Vec2d, uint32> Polygon::triangulated() const {
	PROFILE();
	// Modified version of
	//http://www.flipcode.com/archives/Efficient_Polygon_Triangulation.shtml
	// COTD Entry submitted by John W. Ratcliff [jratcliff@verant.com]

	util::GenericMesh<util::Vec2d, uint32> result;
	util::DynArray<uint32> indices;

	int32 n = poly.size();
	if ( n < 3 ) return result;

	int32 *V = new int[n];

	/* we want a counter-clockwise polygon in V */

	if ( !isClockwise() )
	for (int32 v=0; v<n; v++) V[v] = v;
	else
	for(int32 v=0; v<n; v++) V[v] = (n-1)-v;

	int32 nv = n;

	/*	remove nv-2 Vertices, creating 1 triangle every time */
	int32 count = 2*nv;	  /* error detection */

	for(int32 m=0, v=nv-1; nv>2; )
	{
		/* if we loop, it is probably a non-simple polygon */
		if (0 >= (count--))
		{
			//** Triangulate: ERROR - probable bad polygon!
			print(debug::Ch::General, debug::Vb::Moderate, "Polygon::triangulated(..): Bad polygon");
			result.clear();
			delete [] V;
			return result;
		}

		/* three consecutive vertices in current polygon, <u,v,w> */
		int32 u = v	 ; if (nv <= u) u = 0;	   /* previous */
		v = u+1; if (nv <= v) v = 0;	 /* new v	 */
		int32 w = v+1; if (nv <= w) w = 0;	   /* next	   */

		if ( snip(poly, u, v, w, nv, V) )
		{
			int32 a,b,c,s,t;

			/* true names of the vertices */
			a = V[u]; b = V[v]; c = V[w];

			/* output Triangle */
			result.addIndex( a );
			result.addIndex( b );
			result.addIndex( c );

			m++;

			/* remove v from remaining polygon */
			for(s=v,t=v+1;t<nv;s++,t++) V[s] = V[t]; nv--;

				/* resest error detection counter */
				count = 2*nv;
		}
	}
	delete [] V;

	for (auto& m : poly)
		result.addVertex(m);

	return result;
}

util::DynArray<Polygon> Polygon::splittedToConvex(SizeType max_vert_count) const {
	std::function<util::DynArray<Polygon> (util::Polygon)> partition;
	partition=
		[&] (util::Polygon p) -> util::DynArray<Polygon>
	{
		if (p.getVertexCount() > max_vert_count) {
			util::DynArray<Polygon> ret;
			/// @todo Use better tactic - causes thin polygons
			ret.pushBack(partition(p.splitConvex(p.getVertexCount()/2)));
			ret.pushBack(partition(p));
			return ret;
		} else {
			return {p};
		}
	};

	Polygon p= *this;
	p= p.simplified(0.0);

	TPPLPoly input;
	input.Init(p.poly.size());
	for (SizeType i= 0; i < p.poly.size(); ++i) {
		auto v= p.poly[i];
		input[i].x= v.x;
		input[i].y= v.y;
	}

	// Partition to convex polys using Hertel-Mehlhorn
	std::list<TPPLPoly> outputs;
	TPPLPartition pp;
	pp.ConvexPartition_HM(&input, &outputs);

	util::DynArray<Polygon> convex_polys;
	for (auto&& output_poly : outputs) {
		Polygon poly;
		for (long i= 0; i < output_poly.GetNumPoints(); ++i) {
			poly.append(util::Vec2d{output_poly[i].x, output_poly[i].y});
		}

		if (poly.getVertexCount() > max_vert_count) {
			convex_polys.pushBack(partition(poly));
		} else {
			convex_polys.pushBack(poly);
		}
	}

	return convex_polys;
}

Polygon Polygon::splitConvex(SizeType max_vert_count){
	auto erase_poly_from_this= [this](SizeType begin_i, SizeType size){
		ensure(size <= getVertexCount());

		ensure(size != 0);

		if (size == getVertexCount()){
			clear();
			return;
		}

		if (getVertexCount() == 0)
			return;

		SizeType end_i= (begin_i + size) % getVertexCount();

		Polygon p;
		for (SizeType i= 0; i < getVertexCount() - size + 2; ++i){
			SizeType v_i= (i + begin_i + size - 1) % getVertexCount();
			p.append(getVertex(v_i));
		}

		ensure(p.getVertexCount() + size - 2 == getVertexCount());
		*this= p;
	};

	// Returns true if polygon is fully inside *this
	auto is_good= [this] (const Polygon& p, SizeType begin_i, SizeType size) -> bool {
		ensure(getVertexCount() >= size);

		for (SizeType i= 0; i < getVertexCount() - size; ++i){
			SizeType v_i= (i + begin_i + size) % getVertexCount();

			if (p.isInside(getVertex(v_i)))
				return false;
		}

		return true;
	};

	for (SizeType begin_i= 0; begin_i < getVertexCount(); ++begin_i){
		Polygon convex;

		for (SizeType k= 0; k < getVertexCount(); ++k){
			convex.append(getVertex((k + begin_i) % getVertexCount()));

			if (convex.getVertexCount() == 3){
				if (convex.isClockwise() != isClockwise() || !is_good(convex, begin_i, convex.getVertexCount()))
					break; // Polygon is going to have wrong winding order
			}

			// Polygon is starting to get non-convex
			if (convex.getVertexCount() > 3 && (!convex.isConvex() || !is_good(convex, begin_i, convex.getVertexCount()))){
				convex.popBack();
				ensure(convex.isConvex());
				ensure(is_good(convex, begin_i, convex.getVertexCount()));

				erase_poly_from_this(begin_i, convex.getVertexCount());
				return convex;
			}
			else if (convex.getVertexCount() == max_vert_count || k + 1 == getVertexCount()){
				ensure(is_good(convex, begin_i, convex.getVertexCount()));

				erase_poly_from_this(begin_i, convex.getVertexCount());
				return convex;
			}
		}
	}

	dump();
	release_ensure_msg(0, "Polygon::splitConvex algorithm failed");
	return Polygon{};
}

Polygon Polygon::simplified(real64 min_edge_length_sqr) const {
	auto too_close= [min_edge_length_sqr] (util::Vec2d a, util::Vec2d b) -> bool {
		return (a - b).lengthSqr() <= min_edge_length_sqr;
	};

	Polygon p;
	for (auto v : getVertices()){
		if (	!p.empty() &&
				too_close(p.getVertices().back(), v)){
			continue;
		}

		p.append(v);
	}

	if (	p.getVertexCount() > 1 &&
			too_close(p.getVertices().front(), p.getVertices().back())){
		p.popBack();
	}

	return p;
}

Polygon Polygon::simplified(
		util::Vec2d center, real64 radius, real64 min_edge_length_sqr) const {
	auto too_close= [&] (util::Vec2d a, util::Vec2d b) -> bool {
		return	(a - b).lengthSqr() <= min_edge_length_sqr &&
				(	(a - center).lengthSqr() < radius*radius ||
					(b - center).lengthSqr() < radius*radius);
	};

	Polygon p;
	for (auto v : getVertices()){
		if (	!p.empty() &&
				too_close(p.getVertices().back(), v)){
			continue;
		}

		p.append(v);
	}

	if (	p.getVertexCount() > 1 &&
			too_close(p.getVertices().front(), p.getVertices().back())){
		p.popBack();
	}

	return p;
}

void Polygon::simplify(util::Vec2d center, real64 radius, real64 amount){
	*this= simplified(center, radius, amount);
}

Polygon Polygon::smoothed(
		util::Vec2d center, real64 radius, real64 amount) const {
	PROFILE();
	if (empty())
		return Polygon{};

	const SizeType vcount= getVertexCount();
/*
	// Fully smoothed polygon
	Polygon convoluted= *this;
	for (SizeType i= 0; i < vcount; ++i){
		util::Vec2d conv;
		real64 total_weight= 0.0;
		for (SizeType k= 1; k < vcount; ++k){
			SizeType c= (i + k) % vcount;

			real64 x= getDistance(i, c)/radius;
			real64 weight= pow(10.0, -x);

			conv += getVertex(c)*weight;
			total_weight += weight;
		}

		conv *= 1.0/total_weight;
		convoluted.setVertex(i, conv);
	}
*/	
	// Weights for partially smoothed polygon
	util::DynArray<real64> weights;
	weights.resize(vcount);

	for (SizeType i= 0; i < vcount; ++i){
		util::Vec2d dif= getVertex(i) - center;
		if (dif.lengthSqr() > radius*radius)
			weights[i]= 0.0;
		else
			weights[i]= amount*dif.length()/radius;
	}

	// Partially smoothed polygon
	Polygon smoothed_poly= *this;
	/// @todo Better algorithm
	for (SizeType i= 0; i < vcount; ++i){
		util::Vec2d prev= getVertex((i + vcount - 1) % vcount);
		util::Vec2d cur= getVertex(i);
		util::Vec2d next= getVertex((i + 1) % vcount);
		util::Vec2d fully_smoothed= (prev + next)*0.5;
		util::Vec2d smoothed= fully_smoothed*weights[i] + cur*(1.0 - weights[i]);
		// Don't let vertices outside
		if (isInside(smoothed))
			smoothed_poly.setVertex(i, smoothed);
	}
	return smoothed_poly.simplified(util::epsilon);
}

void Polygon::smooth(util::Vec2d center, real64 radius, real64 amount){
	*this= smoothed(center, radius, amount);
}

Polygon Polygon::offsetted(real64 offset) const {
	PROFILE();
	if (empty())
		return Polygon{};

	ClipperLib::Polygons out_polys;
	ClipperLib::OffsetPolygons({toClipperPoly()}, out_polys, offset*precisionMul);
	ensure(!out_polys.empty());

	SizeType largest_i= 0;
	real64 largest_a= 0.0;
	for (SizeType i= 0; i < out_polys.size(); ++i){
		real64 a= ClipperLib::Area(out_polys[i]);
		if (a > largest_a){
			a= largest_a;
			largest_i= i;
		}
	}

	Polygon offsetted;
	offsetted.fromClipperPoly(out_polys[largest_i]);
	return offsetted.simplified(util::epsilon);
}

void Polygon::offset(real64 offset){
	*this= offsetted(offset);
}


bool Polygon::isClockwise() const {
	real64 sum=0;
	for (auto it= poly.begin(); it!= poly.end(); ++it){
		util::Vec2d a, b;

		if (it == poly.begin()){
			a = *it - poly.back();
		} else a = *it - *(it-1);

		if ((it+1) == poly.end()){
			b = poly.front() - *it;
		} else b= *(it+1) - *it;

		real64 l1,l2;
		l1= a.length();
		l2= b.length();

		if (l1 <= util::epsilon || l2 <= util::epsilon){
			continue;
		}

		real64 cosine= a.dot(b)/l1/l2;
		real64 angle=0;

		if (cosine >= 1 ) angle= 0;
		else if (cosine <= -1) angle= util::pi;
		else angle= acos(cosine);

		if (a.crossZ(b) <= 0) angle= -angle;

		sum += angle;
	}

	if (sum < 0) return true;
	return false;
}

void Polygon::reverse()
{ std::reverse(poly.begin(), poly.end()); }

bool Polygon::isConvex() const {
	if (getVertexCount() < 4)
		return true;

	int winding= 0;

	util::Vec2d last_segment= getVertex(0) - getVertex(getVertexCount() - 1);
	for (SizeType i= 0; i + 1 < getVertexCount(); ++i){
		util::Vec2d segment= getVertex(i + 1) - getVertex(i);
		if (segment.lengthSqr() == 0.0)
			continue;

		int new_winding= last_segment.crossZ(segment) > 0.0 ? 1 : -1;

		if (winding == 0){
			winding= new_winding;
		}
		else if (winding != new_winding){
			return false;
		}

		last_segment= segment;
	}

	return true;
}

bool Polygon::isInside(util::Vec2d point) const {
	int nvert= getVertexCount();
	if (nvert < 3)
		return false;

	bool c= false;
	for (int i= 0, j= nvert-1; i < nvert; j= i++) {
		if (	((getVertex(i).y > point.y) != (getVertex(j).y > point.y)) &&
				(point.x < (getVertex(j).x - getVertex(i).x) * (point.y - getVertex(i).y)/(getVertex(j).y - getVertex(i).y) + getVertex(i).x) )
			c = !c;
	}
	return c;
}

void Polygon::dump() const {
	print(debug::Ch::General, debug::Vb::Trivial, "Polygon vcount: %i", (int32)getVertexCount());
	for (auto v : getVertices()){
		print(debug::Ch::General, debug::Vb::Trivial, "	 v: %f, %f", v.x, v.y);
	}
}

uint32 Polygon::getInsideRectCount(util::Vec2d center, util::Vec2d rad) const {

	uint32 count=0;
	util::Vec2d l= center - rad;
	util::Vec2d r = center + rad;
	for (uint32 i=0; i<poly.size(); ++i){
		util::Vec2d v= poly[i];

		if (v.x >= l.x && v.x < r.x &&
			v.y >= l.y && v.y < r.y){
				++count;
		}
	}

	return count;

}

uint32 Polygon::getInsideRadiusCount(util::Vec2d center, real64 rad) const {
	uint32 count=0;

	for (uint32 i=0; i<poly.size(); ++i){
		util::Vec2d v= poly[i];
		if ( (v - center).lengthSqr() < rad*rad){
				++count;
		}
	}

	return count;
}

bool Polygon::crossesRadius(util::Vec2d center, real64 rad) const {
	for (uint32 i=0; i<poly.size(); ++i){
		util::Vec2d v;
		if (i == poly.size()-1) v= poly[0];
		else v= poly[i+1];

		if ( !util::geom::circleEdgeIntersection(center, rad, poly[i], v).empty() ){
				return true;
		}
	}

	return false;
}

ClipperLib::Polygon Polygon::toClipperPoly() const
{
	ClipperLib::Polygon p;
	p.reserve(poly.size());
	for (auto& v : poly)
		p.push_back(toFixed(v));
	return p;
}

void Polygon::fromClipperPoly(const ClipperLib::Polygon& p)
{
	clear();
	poly.reserve(p.size());
	for (auto& v : p)
		poly.pushBack(toFloating(v));
}

bool Polygon::snip(const util::DynArray<util::Vec2d>& contour, int32 u, int32 v, int32 w, int32 n, int32 *V) const {
	int32 p;
	real64 Ax, Ay, Bx, By, Cx, Cy, Px, Py;

	Ax = contour[V[u]].x;
	Ay = contour[V[u]].y;

	Bx = contour[V[v]].x;
	By = contour[V[v]].y;

	Cx = contour[V[w]].x;
	Cy = contour[V[w]].y;

	if ( util::epsilon > (((Bx-Ax)*(Cy-Ay)) - ((By-Ay)*(Cx-Ax))) ) return false;

	for (p=0;p<n;p++)
	{
		if( (p == u) || (p == v) || (p == w) ) continue;
		Px = contour[V[p]].x;
		Py = contour[V[p]].y;
		if (util::geom::isPointInsideTriangle(util::Vec2d{Px,Py},util::Vec2d{Ax,Ay},util::Vec2d{Bx,By},util::Vec2d{Cx,Cy})) return false;
	}

	return true;
}

PolyClipper::PolyClipper()
	: clipper(new ClipperLib::Clipper())
{ }

PolyClipper::~PolyClipper()
{ delete clipper; }

void PolyClipper::addSubject(const Polygon& p){
	clipper->AddPolygon(p.toClipperPoly(), ClipperLib::ptSubject);
}

void PolyClipper::addSubjects(const util::DynArray<Polygon>& pp){
	for (const auto& p : pp){
		addSubject(p);
	}
}

void PolyClipper::addClipper(const Polygon& p){
	clipper->AddPolygon(p.toClipperPoly(), ClipperLib::ptClip);
}

void PolyClipper::addClippers(const util::DynArray<Polygon>& pp){
	for (const auto& p : pp){
		addClipper(p);
	}
}

void PolyClipper::addCircleClipper(util::Vec2d pos, real64 rad, uint32 ppu){
	Polygon circle;
	uint32 steps= util::tau*rad*ppu;
	for (uint32 i=0; i<steps; ++i){
		real64 angle= util::tau * (real64)i/steps;
		circle.append(pos + util::Vec2d{cos(angle), sin(angle)}*rad );
	}
	addClipper(circle);
}

util::DynArray<Polygon> PolyClipper::execute(ClipType t){
	ClipperLib::ExPolygons solution;
	bool success= clipper->Execute(	(ClipperLib::ClipType)t,
									solution,
									ClipperLib::pftNonZero);
	ensure(success);

	util::DynArray<Polygon> result;
	result.reserve(solution.size());
	for (auto& m : solution){
		Polygon p;
		p.fromClipperPoly(m.outer);
		p.simplify(util::epsilon);
		result.pushBack(std::move(p));
	}
	return result;
}

void PolyClipper::clear()
{ clipper->Clear(); }

} // util
} // clover
