#ifndef CLOVER_UTIL_VECTOR_HPP
#define CLOVER_UTIL_VECTOR_HPP

#include "build.hpp"
#include "ensure.hpp"
#include "hash.hpp"
#include "math_constants.hpp"
#include "math_utils.hpp"

#include <numeric>
#include <cstdlib>
#include <Box2D/Common/b2Math.h>

#define TYPEDEF(N) typedef T VectorElements<T, N>::* const PtrArray[N];

namespace clover {
namespace util {

template <typename T, int N>
struct VectorElements {
protected:
	TYPEDEF(N)
	static const PtrArray v;
};

#define VECTOR_ELEMENTS(N)									\
template <typename T>										\
struct VectorElements<T, N> {								\
protected:													\
	TYPEDEF(N)												\
	static const PtrArray v;								\
public:														\
	T COORDS(EMPTY, N);										\
};															\
template< typename T >										\
const typename 	VectorElements<T, N>::PtrArray				\
				VectorElements<T, N>::v = {					\
					COORDS(WRAP, N)							\
				};


#define EMPTY(a, N) a
#define WRAP(a, N) &VectorElements<T, N>::a

#define COORDS(F, N) F(x, N), F(y, N)
VECTOR_ELEMENTS(2);

#undef COORDS
#define COORDS(F, N) F(x, N), F(y, N), F(z, N)
VECTOR_ELEMENTS(3);

#undef COORDS
#define COORDS(F, N) F(x, N), F(y, N), F(z, N), F(w, N)
VECTOR_ELEMENTS(4);

#undef EMPTY
#undef WRAP
#undef VECTOR_ELEMENTS
#undef COORDS
#undef TYPEDEF

/// @todo Refactor class hierarchy to a single class template

/// CRTP
template <typename T, SizeType N, template <typename, SizeType> class DerivedT>
class GenericVector : public VectorElements<T, N> {
private:
	typedef VectorElements<T, N> Base;
	typedef DerivedT<T, N> Derived;
	typedef GenericVector<T, N, DerivedT> This;
	
protected:
	inline T* begin() { return &v(0); }
	inline const T* begin() const { return &v(0); }
	inline T* end() { return &v(0) + N; }
	inline const T* end() const { return &v(0) + N; }

	inline
	T& v(uint32 i) {
		return this->*Base::v[i];
	}
	
	inline
	const T& v(uint32 i) const{
		return this->*Base::v[i];
	}
	
public:
	typedef T Type;
	static constexpr SizeType dimension= N;

	GenericVector(){
		for (auto m : Base::v)
			this->*m= 0;
	}

	GenericVector(const std::initializer_list<T>& l){
		std::copy(l.begin(), l.end(), &v(0));
	}
	
	GenericVector(const This& other){
		std::copy(other.begin(), other.end(), &v(0));
	}

	GenericVector(const T& value){
		for (auto m : Base::v)
			this->*m= value;
	}
	
	GenericVector(This&& other){
		std::copy(other.begin(), other.end(), &v(0));
	}

	GenericVector(const b2Vec2& other){
		ensure(N == 2);
		v(0)= other.x;
		v(1)= other.y;
	}
	
	T& operator[](uint32 i) {
		return v(i);
	}

    const T& operator[](uint32 i) const {
		return v(i);
	}

	Derived& operator=(Derived&& other){
		std::copy(other.begin(), other.end(), &v(0));
		return *static_cast<Derived*>(this);
	}

	Derived& operator=(const Derived& other){
		std::copy(other.begin(), other.end(), &v(0));
		return *static_cast<Derived*>(this);
	}
	
	bool operator==(const Derived& other) const {
		for (SizeType i= 0; i < N; ++i){
			if (v(i) != other.v(i))
				return false;
		}
		return true;
	}
	
	bool operator!=(const Derived& other) const {
		return !operator==(other);
	}
	
	// Needed?
	/*template <typename T2, typename Derived2>
	This& operator=(const GenericVector<T2, N, Derived2>& v2){
		for (uint32 i=0; i<N; ++i)
			v(i)= (T)v2.v(i);
		
		return *this;
	}*/

	Derived operator-() const {
		This vec= *this;
		vec *= -1;
		return vec;
	}


	Derived& operator+=(const Derived& other){
		for (uint32 i=0; i<N; ++i){
			v(i)+= other.v(i);
		}
		return *static_cast<Derived*>(this);
	}
	
	Derived& operator-=(const Derived& other){
		for (uint32 i=0; i<N; ++i){
			v(i)-= other.v(i);
		}
		return *static_cast<Derived*>(this);
	}

	Derived& operator*=(const T& r){
		for (uint32 i=0; i<N; ++i){
			v(i) = v(i) * r;
		}
		return *static_cast<Derived*>(this);
	}

	Derived& operator*=(const Derived& other){
		for (uint32 i=0; i<N; ++i){
			v(i)*= other.v(i);
		}
		return *static_cast<Derived*>(this);
	}
	
	template<typename U = T>
	Derived& operator/=(const U& r){
		for (uint32 i=0; i<N; ++i){
			v(i) /= r;
		}
		return *static_cast<Derived*>(this);
	}
	
	Derived operator+(const Derived& other) const {
		This t= *this;
		t += other;
		return t;
	}
	
	Derived operator-(const Derived& other) const {
		This t= *this;
		t -= other;
		return t;
	}

	Derived operator*(const Derived& other) const {
		This t= *this;
		t *= other;
		return t;
	}
	
	
	Derived operator/(const T& r) const {
		This t= *this;
		t /= r;
		return t;
	}

	Derived operator*(const T& r) const {
		This t= *this;
		t *= r;
		return t;
	}

	T distanceSqr(const This& other) const {
		T sum=0;
		for (uint32 i=0; i<N; ++i){
			sum += (v(i)-other.v(i))*(v(i)-other.v(i));
		}
		return sum;
	}

	T distance(const This& other) const {
		return sqrt(distanceSqr(other));
	}

	T dot(const Derived& other) const {
		T sum=0;
		for (uint32 i=0; i<N; ++i){
			sum += v(i)*other.v(i);
		}
		
		return sum;
	}
	
	bool componentsGreaterOrEqualThan(const This& other) const {
		for (uint32 i=0; i<N; ++i){
			if (v(i) < other.v(i)) return false;
		}
		return true;
	}
	
	bool componentsGreaterThan(const This& other) const {
		for (uint32 i=0; i<N; ++i){
			if (v(i) <= other.v(i)) return false;
		}
		return true;
	}

	bool componentsLessOrEqualThan(const This& other) const {
		for (uint32 i=0; i<N; ++i){
			if (v(i) > other.v(i)) return false;
		}
		return true;
	}

	bool componentsLessThan(const This& other) const {
		for (uint32 i=0; i<N; ++i){
			if (v(i) >= other.v(i)) return false;
		}
		return true;
	}
	
	template<int Q = 1>
	typename std::enable_if<N == 2 && Q, This>::type rotated(real64 f) const {
		return This{(T)(	v(0)*cos(f) - v(1)*sin(f)), 
					(T)(	v(0)*sin(f) + v(1)*cos(f))};
	}
	
	template <typename T2>
	T2 casted() const {
		T2 vec;
		for (uint32 i=0; i < dimension && i < T2::dimension; ++i)
			vec[i]= (typename T2::Type)v(i);
		return vec;
	}

	template <typename T2>
	T2 converted() const { return casted<T2>(); }
	
	template <typename Archive>
	void serialize(Archive& ar, const uint32 ver){
		for (SizeType i=0; i<N; ++i)
			ar & v(i);
	}
	
	std::array<T, N> asArray() const {
		std::array<T, N> ret;
		for (SizeType i=0; i<N; ++i)
			ret[i]= v(i);
		return ret;
	}

	template <bool Dummy= true>
	typename std::enable_if< N >= 2 && Dummy,
	DerivedT<T, 2>>::type 
	xy() const { return DerivedT<T, 2>{ v(0), v(1) }; }
	
	template <bool Dummy= true>
	typename std::enable_if< N >= 3 && Dummy,
	DerivedT<T, 3>>::type 
	xyz() const { return DerivedT<T, 3>{ v(0), v(1), v(2) }; }

	template <bool Dummy= true>
	typename std::enable_if< N == 2 && Dummy,
	DerivedT<T, 3>>::type 
	xyz() const { return DerivedT<T, 3>{ v(0), v(1), 0.0 }; }

};// __attribute__ ((__packed__));

template <typename T, SizeType N>
class RealVector : public GenericVector<T, N, RealVector>{
private:
	typedef GenericVector<T, N, RealVector> Base;
	typedef RealVector<T, N> This;
	
protected:
	using Base::v;
	using Base::begin;
	using Base::end;
	
public:
	typedef T Value;

	RealVector()= default;
	RealVector(RealVector&&)= default;
	RealVector(const RealVector&)= default;
	
	
	RealVector& operator=(RealVector&& other){
		return Base::operator=(std::move(other));
	}
	
	RealVector& operator=(const RealVector& other){
		return Base::operator=(other);
	}
	
	RealVector(std::initializer_list<T> l):Base(l){
	}

	RealVector(const Base& other):Base(other){
	}
	
	RealVector(const b2Vec2& other){
		debug_ensure(N == 2);
		Base::v(0)= other.x;
		Base::v(1)= other.y;
	}

	RealVector rounded(T step, T offset) const {
		This vec;
		for (uint32 i=0; i<N; ++i){
			vec.v(i) = floor((Base::v(i)+0.5)/step-offset)*step+offset;
		}
		return vec;
	}
	
	RealVector floored() const {
		This vec;
		for (uint32 i=0; i<N; ++i){
			vec.v(i)= floor(Base::v(i));
		}
		return vec;
	}

	template <typename TT>
	IntegerVector<TT, N> discretized() const {
		IntegerVector<TT, N> vec;
		for (uint32 i= 0; i < N; ++i){
			vec[i]= (TT)floor(Base::v(i));
		}
		return vec;
	}

	T crossZ(const RealVector& other) const {
		return Base::v(0)*other.v(1) - Base::v(1)*other.v(0);
	}

	This normal() const {
		for (SizeType i= 0; i < N; ++i){
			for (SizeType k= i + 1; k < N; ++k){
				T v_i= Base::v(i);
				T v_k= Base::v(k);
				if (v_i != 0.0 || v_k != 0.0){
					This n;
					n[i]= -v_k;
					n[k]= v_i;
					return n;
				}
			}
		}

		// This is zero-vector
		ensure(lengthSqr() == 0.0);
		return normalized();
	}

	/// Angle between x-axis
	/// @todo Rename rotationZ
	T rotationZ() const {
		if (Base::v(0)==0.0)
		{
			if (Base::v(1)>0.0)
				return util::pi/2.0;

			return 3*util::pi/2.0;
		}

		if (Base::v(0) < 0.0){
			return util::pi+atan(Base::v(1)/Base::v(0));
		}
		else if (Base::v(1) < 0.0){
			return util::tau - atan(-Base::v(1)/Base::v(0));
		}

		return atan(Base::v(1)/Base::v(0));

	}

	T angleCC(const RealVector& v) const{
		T a1= rotationZ();

		T a2= v.rotationZ();

		if (a2 > a1){
			a1 += 2*util::pi;

		}

	
		return a1-a2;
	}

	T angleC(const RealVector& v) const {

		T a1= rotationZ();
		T a2= v.rotationZ();

		if (a1 > a2){
			a1 -= 2*util::pi;
		}

		return a2-a1;
	}

	bool hasZeroComponent() const {
		for (uint32 i= 0; i < N; ++i){
			if (Base::v(i) == 0.0)
				return true;
		}
		return false;
	}

	This inversed() const {
		This vec;
		for (uint32 i=0; i<N; ++i){
			vec.v(i) =  1.0 / Base::v(i);
		}
		return vec;
	}
	
	template <int Q = 1>
	typename std::enable_if< 	N == 2 && Q &&
								(	std::is_same<T, real32>::value || 
									std::is_same<T, real64>::value ),
	b2Vec2>::type b2() const {
		return b2Vec2(Base::v(0), Base::v(1));
	}

	template<int Q = 1>
	typename std::enable_if<	N == 3 && Q ,
	This>::type cross(const This& o) const {
		return { 	Base::v(1)*o.v(2) - o.v(1)*Base::v(2), 
					o.v(0)*Base::v(2) - Base::v(0)*o.v(2), 
					Base::v(0)*o.v(1) - o.v(0)*Base::v(1) };
	}

	This abs() const {
		This vec;
		for (uint32 i=0; i<N; ++i)
			vec[i]= util::abs(Base::v(i));
		return vec;
	}
	
	T lengthSqr() const {
		T init= 0;
		return std::accumulate(begin(), end(), init, [](const T& v, const T& v2) { return v + v2*v2; });
	}
	
	T length() const {
		return sqrt(lengthSqr());
	}
	
	This normalized() const {
		T q= length();
		if (q==0.0){
	
			This t;
			t.x= 1;
			return t;
		}
		This t= (*this)/q;
		return t;

	}
};

template <typename T, SizeType N>
RealVector<T, N> operator/(T t, RealVector<T, N> vec){
	RealVector<T, N> result;
	for (SizeType i= 0; i < N; ++i)
		result[i]= t/vec[i];
	return result;
}

template <typename T, SizeType N>
class IntegerVector : public GenericVector<T, N, IntegerVector>{
	typedef GenericVector<T, N, IntegerVector> Base;
	typedef IntegerVector<T, N> This;
protected:

	using Base::v;
	using Base::begin;
	using Base::end;
	
public:
	using Value= T;

	IntegerVector()= default;
	IntegerVector(IntegerVector&&)= default;
	IntegerVector(std::initializer_list<T> l): Base(l){
	}
	
	IntegerVector(const Base& other): Base(other){
	}

	IntegerVector(const IntegerVector&)= default;

	IntegerVector& operator=(IntegerVector&& other){
		return Base::operator=(std::move(other));
	}
	
	IntegerVector& operator=(const IntegerVector& other){
		return Base::operator=(other);
	}
	
	template<typename Q>
	operator RealVector<Q, N>() const {
		RealVector<Q, N> vec;
		for (uint32 i=0; i<N; ++i) vec[i]= v(i);
		return vec;
	}
	
	bool operator<(const This& other) const {
		for (uint32 i=0; i<N; ++i){
			if (Base::v(i) != other.v(i)){
				return Base::v(i) < other.v(i);
			}
		}
		return Base::v(N-1) < other.v(N-1);
	}
	
	T lengthSqr() const {
		T init= 0;
		return std::accumulate(begin(), end(), init, [](const T& v, const T& v2) { return v + v2*v2; });
	}
	
};

template <typename T1, SizeType N1, typename T2, SizeType N2>
struct CommonReplaced<RealVector<T1, N1>, RealVector<T2, N2>> {
	using Dst= RealVector<T1, N1>;
	using Src= RealVector<T2, N2>;
	
	static Dst value(Dst dst, Src src){
		Dst v= dst;
		for (SizeType i= 0; i < N1 && i < N2; ++i){
			v[i]= src[i];
		}
		return v;
	}
};

/// Scalar -> Vector
template <typename T1, SizeType N1>
struct CommonReplaced<RealVector<T1, N1>, real32> {
	using Dst= RealVector<T1, N1>;
	using Src= real32;

	static Dst value(Dst dst, Src src){ return Dst((T1)src); }
};

/// Scalar -> Vector
template <typename T1, SizeType N1>
struct CommonReplaced<RealVector<T1, N1>, real64> {
	using Dst= RealVector<T1, N1>;
	using Src= real64;

	static Dst value(Dst dst, Src src){ return Dst((T1)src); }
};

template <typename T, SizeType N>
struct IsFinite<RealVector<T, N>> {
	static bool value(const RealVector<T, N>& v){
		for (SizeType i= 0; i < N; ++i){
			if (!isFinite(v[i]))
				return false;
		}
		return true;
	}
};

template <typename T>
RealVector<T, 2> rotatedBy(const RealVector<T, 2>& vec, const real64& s){ return vec.rotated(s); }

template <typename T, SizeType N>
RealVector<T, N> lerp(RealVector<T, N> v1, RealVector<T, N> v2, real64 f){
	return v1*(1.0 - f) + v2*f;
}

template <typename T, SizeType N>
struct Hash32<RealVector<T, N>> {
	uint32 operator()(const RealVector<T, N>& value) const {
		return rawArrayHash32(value.asArray().data(), N);
	}
};

template <typename T, SizeType N>
struct Hash32<IntegerVector<T, N>> {
	uint32 operator()(const IntegerVector<T, N>& value) const {
		return rawArrayHash32(value.asArray().data(), N);
	}
};


/*
// Kupliksen koodi
  
#include <cstddef>
#include <cstdio>
#include <initializer_list>
#include <array>
#include <algorithm>
#include <cassert>
 
template <typename T, int N>
struct vector_elements
{
        union {
                std::array<T, N> v;
                struct { T x, y, z, w; };
        };
};
 
template <typename T>
struct vector_elements<T, 1>
{
        union {
                std::array<T, 1> v;
                T x;
        };
};
 
template <typename T>
struct vector_elements<T, 2>
{
        union {
                std::array<T, 2> v;
                struct { T x, y; };
        };
};
 
template <typename T>
struct vector_elements<T, 3>
{
        union {
                std::array<T, 3> v;
                struct { T x, y, z; };
        };
};
 
template <typename T, int N>
struct vector : public vector_elements<T, N> {
        typedef vector_elements<T, N> base;
 
        vector(std::initializer_list<T> l)
        {
                std::copy(l.begin(), l.end(), base::v.begin());
        }
 
        vector& operator+=(const vector& o)
        {
                std::transform(o.v.begin(), o.v.end(), base::v.begin(), base::v.begin(), [&](T a, T b) { return a+b; });
                return *this;
        }
 
        vector operator+(const vector<T, N>& o) const
        {
                vector v(*this);
                v += o;
                return v;
        }
 
        T& operator[](std::size_t idx) { assert(idx < N); return base::v[idx]; }
};
 
typedef vector<float, 1> vector1f;
typedef vector<float, 2> vector2f;
typedef vector<float, 3> vector3f;
typedef vector<float, 4> vector4f;
typedef vector<float, 8> vector8f;

int main() {
        vector3f v = {1.1f, 2.2f, 3.3f};
        v += {0, 1, 0};
        v.x += v[2];
        vector3f v2 = v + v;
        printf("(%.1f, %.1f, %.1f)\n", v2.x, v2.y ,v2.z);
}

// Lisää kuplahupsun koodia:

#include <cstdio>
#include <initializer_list>
#include <algorithm>
#include <cassert>
 
template <typename T, std::size_t Rows, std::size_t Cols>
struct matrix_elements
{
        T x;
private:
        T m_rest[Rows*Cols-1];
};
 
template <typename T, std::size_t Rows>
struct matrix_elements<T, Rows, 1>
{
        T x, y, z, w;
private:
        T m_rest[Rows-4];
};
 
template <typename T> struct matrix_elements<T, 1, 1> { T x; };
template <typename T> struct matrix_elements<T, 2, 1> { T x, y; };
template <typename T> struct matrix_elements<T, 3, 1> { T x, y, z; };
template <typename T> struct matrix_elements<T, 4, 1> { T x, y, z, w; };
 
template <typename T, std::size_t Rows, std::size_t Cols>
struct matrix : public matrix_elements<T, Rows, Cols> {
        inline T* begin() { return &this->x; }
        inline const T* begin() const { return &this->x; }
        inline T* end() { return &this->x + Rows * Cols; }
        inline const T* end() const { return &this->x + Rows * Cols; }
 
        matrix()
        {
                std::fill(begin(), end(), T(0));
        }
 
        matrix(std::initializer_list<T> l)
        {
                std::copy(l.begin(), l.end(), begin());
        }
 
        matrix& operator+=(const matrix& o)
        {
                std::transform(o.begin(), o.end(), begin(), begin(), [&](T a, T b) { return a+b; });
                return *this;
        }
 
        matrix operator+(const matrix& o) const
        {
                matrix v(*this);
                v += o;
                return v;
        }
 
        template <std::size_t R = Rows, std::size_t C = Cols>
        static typename std::enable_if<R == 3 && C == 3, matrix>::type makeRotX(T angle)
        {
                T c = std::cos(angle), s = std::sin(angle);
                return {1,  0,  0,
                        0,  c, -s,
                        0,  s,  c};
        }
 
        template <std::size_t Rows2, std::size_t Cols2>
        matrix<T, Rows, Cols2> operator*(const matrix<T, Rows2, Cols2>& o)
        {
                static_assert(Cols == Rows2, "Not valid matrix multiplication");
 
                matrix<T, Rows, Cols2> r;
                for (std::size_t i = 0; i < Cols2; ++i)
                        for (std::size_t j = 0; j < Rows; ++j) {
                                T& t = r(i, j);
                                for (std::size_t k = 0; k < Cols; ++k)
                                        t += (*this)(j, k) * o(k, i);
                        }
 
                return r;
        }
 
        T& operator[](std::size_t idx) { assert(idx < Rows*Cols); return begin()[idx]; }
        const T& operator[](std::size_t idx) const { assert(idx < Rows*Cols); return begin()[idx]; }
 
        T& operator()(std::size_t r, std::size_t c = 0) { return (*this)[r*Cols + c]; }
        const T& operator()(std::size_t r, std::size_t c = 0) const { return (*this)[r*Cols + c]; }
};
 
template <typename T, std::size_t N>
using vector = matrix<T, N, 1>;
 
typedef vector<float, 1> vector1f;
typedef vector<float, 2> vector2f;
typedef vector<float, 3> vector3f;
typedef vector<float, 4> vector4f;
typedef vector<float, 8> vector8f;
 
typedef matrix<float, 3, 3> matrix3f;
 
int main() {
        vector3f v = {1.1f, 2.2f, 3.3f};
        v += {0, 1, 0};
        v.x += v[2];
        vector3f v2 = v + v;
        printf("(%.1f, %.1f, %.1f)\n", v2.x, v2.y, v2.z);
 
        matrix3f r = matrix3f::makeRotX(1.23f);
        matrix3f t = {1,  0,  5,
                      0,  1,  7,
                      0,  0,  1};
        vector3f p = r * t * v2;
        printf("(%.2f, %.2f, %.2f)\n", p.x, p.y, p.z);
 
        matrix<double, 14, 7> test = matrix<double, 14, 19>() * matrix<double, 19, 7>();
}

*/

} // util
} // clover

#endif // CLOVER_UTIL_VECTOR_HPP