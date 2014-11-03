#ifndef CLOVER_UTIL_MATH_UTIL_HPP
#define CLOVER_UTIL_MATH_UTIL_HPP

#include "build.hpp"

#include <cmath>
#include <type_traits>

namespace clover {
namespace util {

/// Traits struct for zeroValue
template <typename T, bool IsPod= std::is_pod<T>::value>
struct ZeroValue;

/// Traits struct for unitValue
template <typename T, bool IsPod= std::is_pod<T>::value>
struct UnitValue;

/// Traits struct for commonReplaced
template <typename Dst, typename Src>
struct CommonReplaced;

/// Traits struct for isFinite
template <typename T>
struct IsFinite;


/// Zero-equivalent value for every type
template <typename T>
T zeroValue(){ return ZeroValue<T>::value(); }

/// Unit-equivalent value for every type
template <typename T>
T unitValue(){ return UnitValue<T>::value(); }

/// Generalization of std::isfinite
template <typename T>
bool isFinite(const T& v){ return IsFinite<T>::value(v); }

/// Returns value where common attributes of dst is replaced by values of src
/// e.g. CommonReplaced<util::Vec2, util::Vec3> replaces xy components
template <typename Dst, typename Src>
Dst commonReplaced(const Dst& dst, const Src& src){
	return CommonReplaced<Dst, Src>::value(dst, src);
}

inline
real64 lerp(real64 r1, real64 r2, real64 f){
	return r1*(1.0 - f) + r2*f;
}

/// Element a rotated by element b
inline
real32 rotatedBy(const real32& a, const real32& b){ return a + b; }

inline
real64 rotatedBy(const real64& a, const real64& b){ return a + b; }

inline
int32 floori(real32 f){ return (int32)floor(f); }

template <typename T>
T abs(const T& t){ return std::abs(t); }

///
/// Default traits
///

template <typename T>
struct ZeroValue<T, true> {	
	static T value(){ return T(0); }
};

template <typename T>
struct ZeroValue<T, false> {
	static T value(){ return T(); }
};


template <typename T>
struct UnitValue<T, true> {	
	static T value(){ return T(1); }
};

template <typename T>
struct UnitValue<T, false> {
	static T value(){ return T(1); }
};

template <typename Dst, typename Src>
struct CommonReplaced {
	static Dst value(const Dst& dst, const Src& src){ Dst ret= dst; ret= src; return ret; }
};

template <typename T>
struct IsFinite {
	static bool value(const T& v){ return std::isfinite(v); }
};

} // util
} // clover

#endif // CLOVER_UTIL_MATH_UTIL_HPP