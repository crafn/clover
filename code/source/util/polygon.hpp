#ifndef CLOVER_UTIL_POLYGON_HPP
#define CLOVER_UTIL_POLYGON_HPP

#include "build.hpp"
#include "dyn_array.hpp"
#include "math.hpp"
#include "mesh.hpp"
#include "transform.hpp"
#include "vector.hpp"

#include <clipper/clipper.hpp>

namespace clover {
namespace util {

class PolyClipper;

/// 2d polygon
/// @todo Support for holes
class Polygon {
public:
	// Doesn't handle tight curves well
	static Polygon extrudedLineSegment(const util::DynArray<util::Vec2d>& points, real64 extrude_length);

	Polygon();

	// Lisää ulkokehään
	void append(util::Vec2d point);
	void append(util::DynArray<util::Vec2d> vertices);
	void popBack();
	void erase(SizeType i);

	void appendRect(util::Vec2d center, util::Vec2d rad);

	void clear();

	void transform(util::RtTransform2d t);
	void translate(util::Vec2d t);
	void rotate(real64 angle);
	void scale(real64 mul);
	void scaleAround(util::Vec2d p, real64 mul);

	// Pätkäsee jokasen edgen kahtia
	void subdivide();

	// Randomoi verteksien sijainteja 0 - amount etäisyydelle
	void addNoise(real64 amount);

	void setVertex(SizeType i, util::Vec2d v);
	util::Vec2d getVertex(SizeType i) const;
	uint32 getVertexCount() const { return poly.outer.size(); }
	bool empty() const { return getVertexCount() == 0; }

	util::DynArray<util::Vec2d> getVertices() const;

	util::Vec2d getCenter() const;
	util::Vec2d getBoundingRadius() const;

	real64 getArea() const;
	/// @return Shortest distance between vertices i and k along polygon
	real64 getDistance(SizeType i, SizeType k) const;

	/// @brief Calculates and returns an indexed mesh from polygon
	util::GenericMesh<util::Vec2d, uint32> triangulated() const;

	/// Returns convex polygons which cover the area of *this
	util::DynArray<Polygon> splittedToConvex(SizeType max_vert_count= (SizeType)-1) const;

	/// Removes a convex polygon from *this and returns it
	Polygon splitConvex(SizeType max_vert_count= (SizeType)-1);

	/// Edges shorter than sqrt(min_edge_length_sqr) are removed
	Polygon simplified(real64 min_edge_length_sqr) const;
	void simplify(real64 min_edge_length_sqr){ *this= simplified(min_edge_length_sqr); }

	Polygon simplified(util::Vec2d center, real64 radius, real64 amount) const;
	void simplify(util::Vec2d center, real64 radius, real64 amount);

	Polygon smoothed(util::Vec2d center, real64 radius, real64 amount) const;
	void smooth(util::Vec2d center, real64 radius, real64 amount);

	Polygon offsetted(real64 offset) const;
	void offset(real64 offset);

	uint32 getInsideRectCount(util::Vec2d center, util::Vec2d rad) const;
	uint32 getInsideRadiusCount(util::Vec2d center, real64 rad) const;

	//bool crossesRect(util::Vec2d center, util::Vec2d rad) const;
	bool crossesRadius(util::Vec2d center, real64 rad) const;


	// Onko koko edgelooppi säteen ulkopuolella
	bool isOutsideRadius(util::Vec2d center, real64 rad) const {
		return !crossesRadius(center, rad) && !getInsideRadiusCount(center, rad);
	}

	bool isClockwise() const;
	void reverse();

	bool isConvex() const;

	bool isInside(util::Vec2d point) const;

	void dump() const;

private:
	// Fixed point multiplier for ClipperLib
	static constexpr real64 precisionMul= 1000000.0;

	util::Vec2d toFloating(const ClipperLib::IntPoint&) const;
	ClipperLib::IntPoint toFixed(const util::Vec2d&) const;
	
	Polygon(ClipperLib::ExPolygon&& p);
	
	friend class PolyClipper;

	// For truangulation
	bool snip(const ClipperLib::Polygon& contour, int32 u, int32 v, int32 w, int32 n, int32 *V) const;

	ClipperLib::ExPolygon poly;
};

class PolyClipper {
public:

	void addSubject(const Polygon& p);
	void addSubjects(const util::DynArray<Polygon>& p);

	void addClipper(const Polygon& p);
	void addClippers(const util::DynArray<Polygon>& p);

	/// ppu: points per unit
	void addCircleClipper(util::Vec2d pos, real64 rad, uint32 ppu=10);

	enum ClipType {
		Intersection= ClipperLib::ctIntersection,
		Union= ClipperLib::ctUnion,
		Difference= ClipperLib::ctDifference,
		Xor= ClipperLib::ctXor
	};

	util::DynArray<Polygon> execute(ClipType t);

	void clear(){ clipper.Clear(); }

private:
	ClipperLib::Clipper clipper;
};

} // util
} // clover

#endif // CLOVER_UTIL_POLYGON_HPP