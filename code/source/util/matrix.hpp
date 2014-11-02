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
	
	static Matrix byRotationAxis(Vec axis, T rotation);
	
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
	
private:
	// Column-major
	std::array<T, N*N> v;
};

/// 'vec' transformed by 'mat'
template <typename T, SizeType N>
RealVector<T, N> operator*(RealVector<T, N> vec, const Matrix<T, N>& mat){
	return mat.transformedVector(vec);
}

} // util
} // clover

#endif // CLOVER_UTIL_MATRIX_HPP