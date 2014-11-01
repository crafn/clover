#include "math.hpp"
#include "debug/debugprint.hpp"
#include "global/exception.hpp"

namespace clover {
namespace util {

util::DynArray<util::Vec2d> geom::circleLineIntersection(util::Vec2d circlepos, real32 radius, util::Vec2d line1, util::Vec2d line2){
    //x = (- kh +- sqrt(r² + k²r² - h²))/(1+k²)

    if (line1.distanceSqr(line2) == 0.0)
		throw Exception("Geometry::circleLineIntersection(..): invalid line");

    // if k == inf, swap x ja y
    if ( abs(line1.x - line2.x) < util::epsilon && abs(line1.y-line2.y) > util::epsilon ){
        util::DynArray<util::Vec2d> points= circleLineIntersection(  {circlepos.y, circlepos.x},
                                                            radius,
                                                            {line1.y,line1.x},
                                                            {line2.y,line2.x});

        util::DynArray<util::Vec2d>::Iter it;
        for (it= points.begin(); it!= points.end(); it++){
            (*it)= util::Vec2d{(*it).y, (*it).x};
        }

        return points;
    }

    real64 k= (line1.y - line2.y)/(line1.x - line2.x);
    real64 b= -k*line1.x+line1.y;
    real64 h= k*circlepos.x+b-circlepos.y;

    util::DynArray<util::Vec2d> points;
    real64 discriminant= radius*radius * (1 + k*k) - h*h;

    if (discriminant < 0) return points;

    if (discriminant < util::epsilon){
		// Tangent case
        real64 x= (-k*h)/(1.0+ k*k);
        points.pushBack( util::Vec2d{x, k*x+h} + circlepos);
    }
    else {
        // Overlapping case
        real64 x1, x2;
        x1= ((-k*h)+sqrt(discriminant))/(1.0+ k*k);
        x2= ((-k*h)-sqrt(discriminant))/(1.0+ k*k);
        points.pushBack( util::Vec2d{x1, k*x1+h} + circlepos);
        points.pushBack( util::Vec2d{x2, k*x2+h} + circlepos);
    }

    return points;
}

util::DynArray<util::Vec2d> geom::circleEdgeIntersection(util::Vec2d circlepos, real32 radius, util::Vec2d edge1, util::Vec2d edge2){
    util::DynArray<util::Vec2d> linepoints= circleLineIntersection(circlepos, radius, edge1, edge2);
    util::DynArray<util::Vec2d> points;
    util::DynArray<util::Vec2d>::Iter it;

    util::Vec2d a,b;

    if (edge1.componentsGreaterThan(edge2)){
        b= edge1;
        a= edge2;
    }
    else if (edge1.x >= edge2.x && edge1.y <= edge2.y){
        a = util::Vec2d{ edge2.x, edge1.y };
        b = util::Vec2d{ edge1.x, edge2.y };
    }
    else if (edge1.x <= edge2.x && edge1.y >= edge2.y){
        a = util::Vec2d{ edge1.x, edge2.y };
        b = util::Vec2d{ edge2.x, edge1.y };
    }
    else {
        a= edge1;
        b= edge2;
    }

    if ( abs(a.x - b.x) < util::epsilon){
        a.x -= 1; b.x += 1;
    }
    else if ( abs(a.y - b.y) < util::epsilon ){
        a.y -= 1; b.y += 1;
    }

    for (it= linepoints.begin(); it != linepoints.end(); it++){
        bool inside= false;

        util::Vec2d p= *it;
        if (p.componentsGreaterOrEqualThan(a) && p.componentsLessOrEqualThan(b) ) {
            inside= true;
        }

        if (inside){
            points.pushBack(*it);
        }
    }

    return points;
}

util::DynArray<util::Vec2d> geom::edgeEdgeIntersection(util::Vec2d a1, util::Vec2d a2, util::Vec2d b1, util::Vec2d b2){
	real64 denominator= (a1.x-a2.x)*(b1.y-b2.y) - (a1.y-a2.y)*(b1.x-b2.x);
	
	if (denominator == 0) return false;
	
	util::Vec2d point= { 	((a1.x*a2.y - a1.y*a2.x)*(b1.x - b2.x) - (a1.x - a2.x)*(b1.x*b2.y - b1.y*b2.x)) / denominator,
					((a1.x*a2.y - a1.y*a2.x)*(b1.y - b2.y) - (a1.y - a2.y)*(b1.x*b2.y - b1.y*b2.x)) / denominator };
	
	
	util::Vec2d left, right;
	
	left= a1;
	right= a1;
	
	if (a2.y < left.y)left.y= a2.y;
	if (a2.x < left.x)left.x= a2.x;
	if (b1.y < left.y)left.y= b1.y;
	if (b1.x < left.x)left.x= b1.x;
	if (b2.y < left.y)left.y= b2.y;
	if (b2.x < left.x)left.x= b2.x;
	
	if (a2.y > right.y)right.y= a2.y;
	if (a2.x > right.x)right.x= a2.x;
	if (b1.y > right.y)right.y= b1.y;
	if (b1.x > right.x)right.x= b1.x;
	if (b2.y > right.y)right.y= b2.y;
	if (b2.x > right.x)right.x= b2.x;
	
	if (point.componentsGreaterOrEqualThan(left) && point.componentsLessThan(right)) return true;
	return false;
}

template <typename T>
bool geom::isPointInsideTriangle(const T& p, const T& a, const T& b, const T& c){
    real64 ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
    real64 cCROSSap, bCROSScp, aCROSSbp;

    ax = c.x - b.x;  ay = c.y - b.y;
    bx = a.x - c.x;  by = a.y - c.y;
    cx = b.x - a.x;  cy = b.y - a.y;
    apx= p.x - a.x;  apy= p.y - a.y;
    bpx= p.x - b.x;  bpy= p.y - b.y;
    cpx= p.x - c.x;  cpy= p.y - c.y;

    aCROSSbp = ax*bpy - ay*bpx;
    cCROSSap = cx*apy - cy*apx;
    bCROSScp = bx*cpy - by*cpx;

    return ((aCROSSbp >= 0.0) && (bCROSScp >= 0.0) && (cCROSSap >= 0.0));
}

template bool geom::isPointInsideTriangle<util::Vec2f>(const util::Vec2f& p, const util::Vec2f& a, const util::Vec2f& b, const util::Vec2f& c);
template bool geom::isPointInsideTriangle<util::Vec2d>(const util::Vec2d& p, const util::Vec2d& a, const util::Vec2d& b, const util::Vec2d& c);

template <typename T>
util::Vec3d geom::barycentric(const T& p, const T& a, const T& b, const T& c){
	util::Vec3d ret= { 	(b.y-c.y)*(p.x-c.x) + (c.x-b.x)*(p.y-c.y),
					(c.y-a.y)*(p.x-c.x) + (a.x-c.x)*(p.y-c.y)};
	
	real64 div= (b.y-c.y)*(a.x-c.x) + (c.x-b.x)*(a.y-c.y);
	ret *= 1.0/div;
	
	ret.z= 1.0 - ret.x - ret.y;
	
	return ret;
}

template util::Vec3d geom::barycentric<util::Vec2f>(const util::Vec2f& p, const util::Vec2f& a, const util::Vec2f& b, const util::Vec2f& c);
template util::Vec3d geom::barycentric<util::Vec2d>(const util::Vec2d& p, const util::Vec2d& a, const util::Vec2d& b, const util::Vec2d& c);

real32 geom::polygonArea(const util::DynArray<util::Vec2d>& contour){
    int32 n = contour.size();

    real64 A=0.0;

    for(int32 p=n-1,q=0; q<n; p=q++)
    {
        A+= contour[p].x*contour[q].y - contour[q].x*contour[p].y;
    }
    return abs(A*0.5);
}

template <typename T>
T geom::pointOnCatmull(real64 t, T p0, T p1, T p2, T p3){
    real64 t2 = t * t;
    real64 t3 = t2 * t;
	using Value= typename T::Value;
    return T 	{ 	(Value)	(0.5 * ( ( 2.0 * p1.x ) +
							( -p0.x + p2.x ) * t +
							( 2.0 * p0.x - 5.0 * p1.x + 4 * p2.x - p3.x ) * t2 +
							( -p0.x + 3.0 * p1.x - 3.0 * p2.x + p3.x ) * t3 )),

					(Value)	(0.5 * ( ( 2.0 * p1.y ) +
							( -p0.y + p2.y ) * t +
							( 2.0 * p0.y - 5.0 * p1.y + 4 * p2.y - p3.y ) * t2 +
							( -p0.y + 3.0 * p1.y - 3.0 * p2.y + p3.y ) * t3 ))

    };
}

template util::Vec2f geom::pointOnCatmull<util::Vec2f>(real64 t, util::Vec2f p0, util::Vec2f p1, util::Vec2f p2, util::Vec2f p3);
template util::Vec2d geom::pointOnCatmull<util::Vec2d>(real64 t, util::Vec2d p0, util::Vec2d p1, util::Vec2d p2, util::Vec2d p3);

boost::mt19937 Rand::mersenne;

int32 limitUC(int32 i){
    if (i > 255)
        return 255;

    if (i < 0)
        return 0;

    return i;
}

int32 limitC(int32 i){
    if (i > 127)
        return 127;

    if (i < -127)
        return -127;

    return i;
}

bool isPointInFrontOfLine(util::Vec2d point, util::Vec2d line_anchor, util::Vec2d normal) {
    util::Vec2d a= point - line_anchor;

    real64 b= a.dot(normal);

    if (b > 0.0) return true;

    return false;

}
util::Vec2d closestPointOnLineSegment(util::Vec2d point, util::Vec2d a, util::Vec2d b){
    util::Vec2d A= point - a;

    util::Vec2d ab= b-a;
    ab= ab.normalized();

    util::Vec2d closest_point= ab*A.dot(ab) + a;

    return closest_point;
}

real32 normalizedAngle(real32 angle){
    while(angle > util::tau || angle < 0){
        if (angle < 0)angle += util::tau;
        else angle -= util::tau;
    }
    return angle;
}

real32 angleBetweenCW(real32 angle1, real32 angle2){
    angle1= normalizedAngle(angle1);
    angle2= normalizedAngle(angle2);

    if (angle2 < angle1){
        return util::tau + (angle2-angle1);
    }

    return angle2-angle1;
}

real32 angleBetweenCCW(real32 angle1, real32 angle2){
    angle1= normalizedAngle(angle1);
    angle2= normalizedAngle(angle2);

    if (angle2 > angle1){
        return util::tau - (angle2-angle1);
    }

    return angle1-angle2;
}

real32 smallestAngleBetween(real32 angle1, real32 angle2){
    angle1= normalizedAngle(angle1);
    angle2= normalizedAngle(angle2);

    real32 mul=1;

    if (angleBetweenCW(angle1, angle2) < angleBetweenCCW(angle1, angle2))
        mul= -1;

    real32 a1= abs(angle1-angle2);
    real32 a2= abs(angle1-angle2+util::tau);
    real32 a3= abs(angle1-angle2-util::tau);

    if (a1 < a2 && a1 < a3)
        return a1*mul;

    if (a2 < a1 && a2 < a3)
        return a2*mul;

    if (a3 < a1 && a3 < a2)
        return a3*mul;

    return 0;
}

} // util
} // clover