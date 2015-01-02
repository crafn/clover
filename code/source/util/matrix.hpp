#ifndef CLOVER_UTIL_MATRIX_HPP
#define CLOVER_UTIL_MATRIX_HPP

#include "build.hpp"
#include "vector.hpp"

#include <array>

namespace clover {
namespace util {

/// A matrix which follows a convention of transforming by post-multiplying
template <typename T, SizeType N>
struct Matrix {
	using Vec= RealVector<T, N>;
	
	static Matrix identity();
	
	static Matrix byScale(RealVector<T, N-1> s);
	static Matrix byRotationAxis(Vec axis, T rotation);
	static Matrix byTranslation(RealVector<T, N-1> t);
	
	/// Constructs a zero-matrix
	Matrix();
	
	/// *this transformed by other
	Matrix operator*(const Matrix& other) const;

	T& operator()(SizeType x, SizeType y);
	T operator()(SizeType x, SizeType y) const;
	
	Vec row(SizeType y) const;
	Vec column(SizeType x) const;
	
	Matrix transpose() const;
	
	T* data(){ return v.data(); }
	const T* data() const { return v.data(); }

	void setBvhEulerRotation(Vec euler_angles);

	void print();
	
	/// Helper for operator* defined outside class
	Vec transformedVector(Vec vec) const;

	template <SizeType M>
	Matrix applied(const Matrix<T, M>& other) const;

private:
	// Column-major
	std::array<T, N*N> v;
};

typedef Matrix<real32, 3> Mat33f;
typedef Matrix<real32, 4> Mat44f;

typedef Matrix<real64, 3> Mat33d;
typedef Matrix<real64, 4> Mat44d;

/// 'vec' transformed by 'mat'
template <typename T, SizeType N>
RealVector<T, N> operator*(RealVector<T, N> vec, const Matrix<T, N>& mat)
{ return mat.transformedVector(vec); }

template <typename T, SizeType N>
template <SizeType M>
Matrix<T, N> Matrix<T, N>::applied(const Matrix<T, M>& other) const
{
	auto result= *this;
	for (SizeType y= 0; y < M && y < N; ++y) {
		for (SizeType x= 0; x < M && x < N; ++x) {
			result(x, y)= other(x, y);
		}
	}
	return result;
}

} // util
} // clover

#endif // CLOVER_UTIL_MATRIX_HPP
