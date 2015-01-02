#ifndef CLOVER_UTIL_QUATERNION_HPP
#define CLOVER_UTIL_QUATERNION_HPP

#include "build.hpp"
#include "hash.hpp"
#include "math_utils.hpp"
#include "matrix.hpp"

namespace clover {
namespace util {

template <typename T>
class Quaternion {
public:
	using Value= T;
	using Vec= RealVector<T, 3>;
	
	static Quaternion identity();
	
	// Rotation between vectors
	static Quaternion byRotationFromTo(const Vec& v1_, const Vec& v2_);

	static Quaternion byRotationAxis(const Vec& axis_, T rotation);
	
	Quaternion();
	Quaternion(T x_, T y_, T z_, T w_);
	
	Quaternion& operator=(const Quaternion& q)= default;
	
	// Rotation from matrix
	Quaternion& operator=(const Matrix<T, 3>& m);
	
	Quaternion operator+(const Quaternion& q) const;
	/// @return Opposite rotation to match semantics of scalar angles
	/// @note Use other() to get the same rotation with opposite axis
	Quaternion operator-() const;
	Quaternion& operator+=(const Quaternion& q);
	Quaternion operator/(const T& v) const;
	Quaternion& operator/=(const T& v);
	Quaternion operator*(const T& v) const;
	Quaternion& operator*=(const T& v);
	Quaternion operator*(const Quaternion& q) const;
	Quaternion& operator*=(const Quaternion& q);
	
	bool operator==(const Quaternion& q) const;

	Quaternion other() const;
	Quaternion normalized() const;
	T rotation() const;
	T dot(const Quaternion& q) const;
	Vec xyz() const;
	Vec axis() const;
	T rotationZ() const;
	
	Matrix<T, 3> asMatrix() const;
	
	template <typename R>
	R casted() const { return R(x, y, z, w); }
	
	template <typename Archive>
	void serialize(Archive& ar, const uint32 ver){
		ar & x;
		ar & y;
		ar & z;
		ar & w;
	}
	
	T x, y, z, w;
};

typedef Quaternion<real32> Quatf;
typedef Quaternion<real64> Quatd;

/// Rotate vector with quaternion
template <typename T>
RealVector<T, 3> operator*(RealVector<T, 3> vec, Quaternion<T> q){
	RealVector<T, 3> a, b, c{q.x, q.y, q.z};
	a= c.cross(vec);
	b= c.cross(a);
	a *= 2.0 * q.w;
	b *= 2.0;
	return vec + a + b;
}

/// When real number is commonReplaced by Quaternion, the number is considered to be rotation around z-axis
/// @todo Consider using some Rotation-type instead of plain numbers
template <typename T>
struct CommonReplaced<real64, Quaternion<T>> {
	using Dst= real64;
	using Src= Quaternion<T>;
	
	static Dst value(Dst dst, Src src){
		return (Dst)src.rotationZ();
	}
};

template <typename T>
struct CommonReplaced<real32, Quaternion<T>> {
	using Dst= real32;
	using Src= Quaternion<T>;
	
	static Dst value(Dst dst, Src src){
		return (Dst)commonReplaced((real64)dst, src);
	}
};

/// Rotation around z-axis to Quaternion
/// @todo Consider using some Rotation-type instead of plain numbers
template <typename T>
struct CommonReplaced<Quaternion<T>, real64> {
	using Dst= Quaternion<T>;
	using Src= real64;
	
	static Dst value(Dst dst, Src src){
		return Dst::byRotationAxis({0.0, 0.0, 1.0}, src);
	}
};

template <typename T>
struct CommonReplaced<Quaternion<T>, real32> {
	using Dst= Quaternion<T>;
	using Src= real32;
	
	static Dst value(Dst dst, Src src){
		return commonReplaced(dst, (real64)src);
	}
};

/// Quaternion to other type of Quaternion
template <typename T1, typename T2>
struct CommonReplaced<Quaternion<T1>, Quaternion<T2>> {
	using Dst= Quaternion<T1>;
	using Src= Quaternion<T2>;

	static Dst value(Dst dst, Src src){
		return src.template casted<Dst>();
	}
};

template <typename T>
struct IsFinite<Quaternion<T>> {
	static bool value(const Quaternion<T>& v){
		return
			isFinite(v.x) &&
			isFinite(v.y) &&
			isFinite(v.z) &&
			isFinite(v.w);
	}
};


template <typename T>
Quaternion<T> lerp(Quaternion<T> q1, Quaternion<T> q2, real64 f);

template <typename T>
Quaternion<T> rotatedBy(const Quaternion<T>& a, const Quaternion<T>& b){ return a*b; }

template <typename T>
RealVector<T, 3> rotatedBy(const RealVector<T, 3>& vec, const Quaternion<T>& q){ return vec*q; }

template <typename T>
struct Hash32<Quaternion<T>> {
	uint32 operator()(const Quaternion<T>& q) const {
		std::array<T, 4> arr= { q.x, q.y, q.z, q.w };
		return rawArrayHash32(arr.data(), arr.size());
	}
};

/// Throws exception if Quaternion behaves incorrectly
void quaternionUnitTest();

} // util
} // clover

#endif // CLOVER_UTIL_QUATERNION_HPP
