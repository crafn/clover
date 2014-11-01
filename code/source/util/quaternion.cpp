#include "quaternion.hpp"
#include "debug/debugprint.hpp"
#include "ensure.hpp"
#include "util/string.hpp"

namespace clover {
namespace util {

template <typename T>
Quaternion<T> Quaternion<T>::identity(){
	return Quaternion(0, 0, 0, 1);
}

template <typename T>
Quaternion<T> Quaternion<T>::byRotationFromTo(const Vec& v1_, const Vec& v2_){
	auto v1= v1_, v2= v2_;
	
	v1= v1.normalized(); v2= v2.normalized();

	T dot= v1.dot(v2);
	
	if (dot >= 1.0){
		return identity();
	}
	else if (dot <= -1.0){
		
		Vec axis= {1.0, 0.0, 0.0};
		axis= axis.cross(v1);
		if (axis.lengthSqr() == 0.0){
			axis= Vec{0.0, 1.0, 0.0};
			axis= axis.cross(v1);
		}
		
		return Quaternion(axis.x, axis.y, axis.z, 0).normalized();
	}

	real32 mul= sqrt(2 + dot*2);
	
	Vec v= v1.cross(v2)/mul;
	return Quaternion(v.x, v.y, v.z, 0.5*mul).normalized(); // Normalisaatio ei välttis pakollinen
}

template <typename T>
Quaternion<T> Quaternion<T>::byRotationAxis(const Vec& axis_, T rotation){
	Vec axis= axis_.normalized();
	T half= 0.5*rotation;
	T s= sin(half);
	
	return Quaternion(axis.x*s, axis.y*s, axis.z*s, cos(half)).normalized();
}

template <typename T>
Quaternion<T>::Quaternion():
	x(0), y(0), z(0), w(1){
}

template <typename T>
Quaternion<T>::Quaternion(T x_, T y_, T z_, T w_):
	x(x_), y(y_), z(z_), w(w_) {
}

template <typename T>
Quaternion<T>& Quaternion<T>::operator=(const Matrix<T, 3>& m){
	real32 d= m(0,0) + m(1,1) + m(2,2) + 1;
	
	real32 	a= m(1,0) - m(0,1),
			b= m(0,2) - m(2,0),
			c= m(2,1) - m(1,2);
	
	if (d > 0.0){
		real32 mul= 2 / sqrt(d);
		
		x= mul*c;
		y= mul*b;
		z= mul*a;
		w= 0.25/mul;
	}
	else {
		if (m(0,0) > m(1,1) && m(0,0) > m(2,2)){
			real32 mul= 2 / sqrt(m(0,0) - m(1,1) - m(2,2) + 1);
			
			x= 0.25/mul;
			y= mul*a;
			z= mul*b;
			w= mul*c;
			
		}
		else if (m(1,1) > m(2,2)){
			real32 mul= 2 / sqrt(m(1,1) - m(0,0) - m(2,2) + 1);
			
			x= mul*a;
			y= 0.25/mul;
			z= mul*c;
			w= mul*b;
			
		}
		else {
			real32 mul= 2 / sqrt(m(2,2) - m(0,0) - m(1,1) + 1);
			
			x= mul*b;
			y= mul*c;
			z= 0.25/mul;
			w= mul*a;
			
		}
	}
	
	return *this= normalized();
}

template <typename T>
Quaternion<T> Quaternion<T>::operator+(const Quaternion& q) const {
	return Quaternion(x+q.x, y+q.y, z+q.z, w+q.w);
}

template <typename T>
Quaternion<T> Quaternion<T>::operator-() const {
	return Quaternion(-x, -y, -z, w);
}

template <typename T>
Quaternion<T>& Quaternion<T>::operator+=(const Quaternion& q) {
	return *this = *this+q;
}

template <typename T>
Quaternion<T> Quaternion<T>::operator/(const T& v) const {
	return Quaternion(x/v, y/v, z/v, w/v);
}

template <typename T>
Quaternion<T>& Quaternion<T>::operator/=(const T& v) {
	return *this= *this / v;
}

template <typename T>
Quaternion<T> Quaternion<T>::operator*(const T& v) const {
	return Quaternion(x*v, y*v, z*v, w*v);
}

template <typename T>
Quaternion<T>& Quaternion<T>::operator*=(const T& v) {
	return *this= *this*v;
}

template <typename T>
Quaternion<T> Quaternion<T>::operator*(const Quaternion& q) const {
	
	return Quaternion(	q.w*x + q.x*w + q.y*z - q.z*y,
						q.w*y + q.y*w + q.z*x - q.x*z,
						q.w*z + q.z*w + q.x*y - q.y*x, 
						q.w*w - q.x*x - q.y*y - q.z*z).normalized();
}

template <typename T>
Quaternion<T>& Quaternion<T>::operator*=(const Quaternion& q) {
	return *this= *this*q;
}

template <typename T>
bool Quaternion<T>::operator==(const Quaternion& q) const {
	return x == q.x && y == q.y && z == q.z && w == q.w;
}

template <typename T>
Quaternion<T> Quaternion<T>::other() const {
	return (*this)*(-1);
}

template <typename T>
Quaternion<T> Quaternion<T>::normalized() const {
	const T a = x*x + y*y + z*z + w*w;
	
	if (a == 1)
		return *this;
	else if (a == 0)
		return identity();
	
	
	return *this / sqrt(a);

}

template <typename T>
T Quaternion<T>::rotation() const {
	return 2.0*acos(w);
}

template <typename T>
T Quaternion<T>::dot(const Quaternion& q) const {
	return x*q.x + y*q.y + z*q.z + w*q.w;
}

template <typename T>
auto Quaternion<T>::xyz() const -> Vec {
	return Vec{x, y, z};
}

template <typename T>
auto Quaternion<T>::axis() const -> Vec {
	T s= sqrt(x*x + y*y + z*z);
	if (s <= util::epsilon || w > 1.0 || w < -1.0){
		return Vec{0, 1, 0};
	}
	else {
		T inv_s= 1.0/s;
		return Vec{x*inv_s, y*inv_s, z*inv_s};
	}
}

template <typename T>
T Quaternion<T>::rotationZ() const {

	/// @note	If fixing is needed, make sure that these still work:
	///			 - physics object mirroring (e.g. stoneFlail in hand)
	///			 - full rotation in 2d plane (e.g. grassClump attached to object)

	bool flip= (Vec{0, 0, 1}*(*this)).z < 0;

	// Euler angles
	real64 heading= atan2(2.0*y*w - 2.0*x*z, 1.0 - 2.0*y*y - 2.0*z*z);
	real64 attitude= asin(2.0*x*y + 2.0*z*w);

	// Using heading to detect rotations greater than +-90 degrees
	// Flipping was adjusted by trial & error
	if (abs(heading) < util::pi*0.5 || flip)
		return flip ? -attitude : attitude;
	else
		return util::pi - attitude;
}

template <typename T>
Matrix<T, 3> Quaternion<T>::asMatrix() const {
	return Matrix<T, 3>::byRotationAxis(axis(), rotation());
}

template <typename T>
Quaternion<T> lerp(Quaternion<T> q1, Quaternion<T> q2, real64 f){
	if (q1.dot(q2) < 0){
		// Pick the shorter path between rotations
		q2= q2.other();
	}
	return q1*(1.0 - f) + q2*f;
}

template util::Quatf lerp(util::Quatf q1, util::Quatf q2, real64 f);
template util::Quatd lerp(util::Quatd q1, util::Quatd q2, real64 f);

template class Quaternion<real32>;
template class Quaternion<real64>;

struct Tester { Tester(){ quaternionUnitTest(); } };
//Tester g_tester;

void singleTest(const util::Str8& name, const util::Quatd& q, util::Vec3d axis, real64 rot){
	if (q.axis().dot(axis) < 0.09)
		throw Exception("quaternionUnitTest: %s, invalid axis: {%f, %f, %f} != {%f, %f, %f}",
						name.cStr(), q.axis().x, q.axis().y, q.axis().z, axis.x, axis.y, axis.x);
	else if (abs(q.rotation() - rot) > 0.01)
		throw Exception("quaternionUnitTest: %s, invalid rotation: %f != %f", name.cStr(), q.rotation(), rot);
}

void quaternionUnitTest(){

	ensure((util::Vec3d{1.0, 1.0, 1.0}.xy() == util::Vec2d{1.0, 1.0}));
	ensure((util::Vec2d{1.0, 1.0}.xyz() == util::Vec3d{1.0, 1.0, 0.0}));
	
	Quaternion<real64> q= Quaternion<real64>::byRotationAxis({0.0, 0.0, 1.0}, -util::pi);
	singleTest("constructByRotationAxis", q, util::Vec3d{0.0, 0.0, -1.0}, util::pi);

	q *= Quaternion<real64>::byRotationAxis({0.0, 0.0, 1.0}, util::pi/2.0);
	singleTest("multiplication", q, util::Vec3d{0.0, 0.0, -1.0}, 1.0/2.0*util::pi);

	q= Quaternion<real64>::byRotationFromTo(util::Vec3d{1.0, 0.0, 0.0}, util::Vec3d{0.0, 1.0, 0.0});
	singleTest("rotationFromTo", q, util::Vec3d{0.0, 0.0, 1.0}, util::pi/2.0);
	
	{
		util::Vec3d v{2.0, 0.0, 0.0};
		v= v*Quaternion<real64>::byRotationAxis(util::Vec3d{0.0, 0.0, 1.0}, util::pi/2.0);
		singleTest("Vector rot", Quaternion<real64>::byRotationAxis(v, 1.0), {0.0, 2.0, 0.0}, 1.0);
	}

	{
		real64 rot= util::Quatd::byRotationFromTo(util::Vec3d{1, 0, 0}, util::Vec3d{-0.7, 0.0, -0.7}).
			   rotationZ();
		if (abs(rot) < util::pi - 0.01 || abs(rot) > util::pi + 0.01)
			throw Exception("util::Quatd rotationZ test 1 failed with rot: %f", rot);
	}
	{
		real64 rot= util::Quatd::byRotationFromTo(util::Vec3d{0, 1, 0}, util::Vec3d{0.0, -0.7, 0.7}).
			   rotationZ();
		if (abs(rot) < util::pi - 0.01 || abs(rot) > util::pi + 0.01)
			throw Exception("util::Quatd rotationZ test 2 failed with rot: %f", rot);
	}
}

} // util
} // clover