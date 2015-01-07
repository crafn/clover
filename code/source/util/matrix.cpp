#include "matrix.hpp"
#include "util/string.hpp"
#include "debug/print.hpp"

namespace clover {
namespace util {

template <typename T, SizeType N>
Matrix<T, N> Matrix<T, N>::identity()
{
	Matrix<T, N> m;
	for (SizeType i= 0; i < N; ++i){
		m(i, i)= 1;
	}
	return m;
}

template <typename T, SizeType N>
Matrix<T, N> Matrix<T, N>::byScale(RealVector<T, N-1> s)
{
	auto result= Matrix<T, N>::identity();
	for (SizeType i= 0; i < N-1; ++i)
		result(i, i)= s[i];
	return result;
}

template <typename T, SizeType N>
Matrix<T, N> Matrix<T, N>::byRotationAxis(Vec axis, T rotation)
{
	ensure(N >= 3);
	
	// From wikipedia
	
	T x, y, z, c, s, C;
	
	x= axis.x;
	y= axis.y;
	z= axis.z;
	c= cos(rotation);
	s= sin(rotation);
	C= 1 - c;
	
	Matrix<T, N> result;
	result(0,0)= x*x*C + c;
	result(0,1)= x*y*C - z*s;
	result(0,2)= x*z*C + y*s;
	
	result(1,0)= y*x*C + z*s;
	result(1,1)= y*y*C + c; 
	result(1,2)= y*z*C - x*s;
	
	result(2,0)= z*x*C - y*s;
	result(2,1)= z*y*C + x*s;
	result(2,2)= z*z*C + c;
	
	return result;
}

template <typename T, SizeType N>
Matrix<T, N> Matrix<T, N>::byTranslation(RealVector<T, N-1> t)
{
	auto result= Matrix::identity();
	for (SizeType i= 0; i < N-1; ++i) {
		result(i, N-1)= t[i];
	}
	return std::move(result);
}

template <typename T, SizeType N>
Matrix<T, N>::Matrix(){
	for(SizeType i= 0; i < N*N; ++i)
		v[i]= 0;
}

template <typename T, SizeType N>
Matrix<T, N> Matrix<T, N>::operator*(const Matrix<T, N>& mat) const
{
	Matrix<T, N> result;
	
	
	for (SizeType x= 0; x < N; ++x){
		for (SizeType y= 0; y < N; ++y){
			result(x,y)= row(y).dot(mat.column(x)); 
		}
	}

	return result;
}

template <typename T, SizeType N>
T& Matrix<T, N>::operator()(SizeType x, SizeType y)
{
	return v[x*N + y];
}

template <typename T, SizeType N>
T Matrix<T, N>::operator()(SizeType x, SizeType y) const
{
	return v[x*N + y];
}

template <typename T, SizeType N>
auto Matrix<T, N>::row(SizeType y) const -> Vec
{
	RealVector<T, N> result;
	for (SizeType x= 0; x < N; ++x){
		result[x]= (*this)(x, y);
	}
	return result;
}

template <typename T, SizeType N>
auto Matrix<T, N>::column(SizeType x) const -> Vec
{
	RealVector<T, N> result;
	for (SizeType y= 0; y < N; ++y){
		result[y]= (*this)(x, y);
	}
	return result;
}

template <typename T, SizeType N>
Matrix<T, N> Matrix<T, N>::transpose() const
{
	Matrix result= *this;
	for (SizeType x= 0; x < N; ++x){
		for (SizeType y= 0; y < N; ++y){
			std::swap(result(x,y), result(y,x));
		}
	}
	return result;
}

template <typename T, SizeType N>
void Matrix<T, N>::setBvhEulerRotation(Vec rot)
{
	ensure(N >= 3);
	
	T x, y, z;
	x= rot.x;
	y= rot.y;
	z= rot.z;

	Matrix<T, N> X, Y, Z;
	X= Y= Z= Matrix<T, N>::identity();
	
	X(1,1)= cos(x); X(2,1)= -sin(x);
	X(1,2)= sin(x); X(2,2)= cos(x);

	Y(0,0)= cos(y);	 Y(2,0)= sin(y);
	Y(0,2)= -sin(y); Y(2,2)= cos(y);

	Z(0,0)= cos(z); Z(1,0)= -sin(z);
	Z(0,1)= sin(z); Z(1,1)= cos(z);

	*this = Y*Z*X;
}

template <typename T, SizeType N>
auto Matrix<T, N>::transformedVector(RealVector<T, N> vec) const -> Vec
{
	Vec result;
	for (SizeType i= 0; i < N; ++i){
		result[i]= row(i).dot(vec);
	}
	return result;
}

template <typename T, SizeType N>
void Matrix<T, N>::print()
{
	util::Str8 str;
	for (SizeType y= 0; y < N; y++){
		for (SizeType x= 0; x < N; x++){
			str += util::Str8::format("%f ", (*this)(x, y));
		}
		str += "\n";
	}
	
	debug::print(debug::Ch::General, debug::Vb::Trivial, "Matrix: %s", str.cStr());
}

template class Matrix<real32, 3>;
template class Matrix<real32, 4>;

template class Matrix<real64, 3>;
template class Matrix<real64, 4>;

} // util
} // clover
