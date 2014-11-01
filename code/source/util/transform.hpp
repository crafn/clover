#ifndef CLOVER_UTIL_TRANSFORM_HPP
#define CLOVER_UTIL_TRANSFORM_HPP

#include "build.hpp"
#include "hash.hpp"
#include "math_utils.hpp"

#include <utility>

namespace clover {
namespace util {

/// Position
template <typename T>
struct TTransform {
	typedef T Translation;
	TTransform():translation(zeroValue<T>()){}
	TTransform(const T& t):translation(t){}

	TTransform(const TTransform&)= default;
	TTransform(TTransform&&)= default;
	TTransform& operator=(const TTransform&)= default;
	TTransform& operator=(TTransform&&)= default;
	
	bool operator==(const TTransform& other) const { return translation == other.translation; }
	bool operator!=(const TTransform& other) const { return !operator==(other); }
	
	T translation;
};

/// Position, Rotation
template <typename R, typename T>
struct RtTransform : public TTransform<T> {
	typedef TTransform<T> Base;
	typedef RtTransform<R, T> This;
	typedef R Rotation;
	
	RtTransform():rotation(zeroValue<R>()){}
	RtTransform(const R& r, const T& t): TTransform<T>(t), rotation(r){}

	RtTransform(const RtTransform&)= default;
	RtTransform(RtTransform&&)= default;
	RtTransform& operator=(const RtTransform&)= default;
	RtTransform& operator=(RtTransform&&)= default;

	bool operator==(const RtTransform& other) const {
		return Base::operator==(other) && rotation == other.rotation;
	}
	
	bool operator!=(const RtTransform& other) const { return !operator==(other); }
	
	template <typename RR, typename TT>
	void commonReplaced(const RtTransform<RR, TT>& t){ return commonReplaced(*this, t); }
	
	/// @return *this transformed by other in order: rotate -> translate
	This operator*(const This& other) const {
		This t= *this;
		t.rotation= rotatedBy(t.rotation, other.rotation);
		t.translation= rotatedBy(t.translation, other.rotation) + other.translation;
		return t;
	}
	
	/// transform * transform.inversed() == identity
	This inversed() const {
		This t= *this;
		t.rotation= -t.rotation;
		t.translation= -rotatedBy(t.translation, t.rotation);
		return t;
	}
	
	template <typename Archive>
	void serialize(Archive& ar, const uint32& ver){
		ar & Base::translation;
		ar & rotation;
	}
	
	R rotation;
};


/// Position, Rotation, Scale
template <typename S, typename R, typename T>
struct SrtTransform : public RtTransform<R, T> {
	typedef SrtTransform<S, R, T> This;
	typedef RtTransform<R, T> Base;
	typedef S Scale;
	typedef R Rotation;
	typedef T Translation;
	
	SrtTransform():scale(unitValue<S>()){}
	SrtTransform(const S& s, const R& r, const T& t): RtTransform<R,T>(r, t), scale(s){}

	SrtTransform(const SrtTransform&)= default;
	SrtTransform(SrtTransform&&)= default;
	SrtTransform& operator=(const SrtTransform&)= default;
	SrtTransform& operator=(SrtTransform&&)= default;

	bool operator==(const SrtTransform& other) const {
		return Base::operator==(other) && scale == other.scale;
	}
	
	bool operator!=(const SrtTransform& other) const { return !operator==(other); }
	
	template <typename SS, typename RR, typename TT>
	void commonReplaced(const SrtTransform<SS, RR, TT>& t){ return commonReplaced(*this, t); }
	
	Base rtTransform() const { return Base(Base::rotation, Base::translation); }
	
	/// transform * transform.inversed() == identity
	This inversed() const {
		This t= *this;
		t.scale= 1.0/t.scale;
		t.rotation= -t.rotation;
		t.translation= -rotatedBy(t.translation*t.scale, t.rotation);
		return t;
	}

	/// @return *this transformed by other in order: scale -> rotate -> translate
	This operator*(const This& other) const {
		This t= *this;
		t.scale= t.scale * other.scale;
		t.rotation= rotatedBy(t.rotation, other.rotation);
		t.translation= rotatedBy(t.translation*other.scale, other.rotation) + other.translation;
		return t;
	}
	

	This& operator*=(const This& other){
		*this= *this * other;
		return *this;
	}

	template <typename Archive>
	void serialize(Archive& ar, const uint32& ver){
		ar & Base::translation;
		ar & Base::rotation;
		ar & scale;
	}
	
	S scale;
};

/// Transforming of a vector
template <typename R, typename T>
typename RtTransform<R, T>::Translation operator*(const typename RtTransform<R, T>::Translation& v, const RtTransform<R, T>& t){
	return (RtTransform<R, T>(zeroValue<R>(), v)*t).translation;
}

template <typename R, typename T>
RtTransform<R, T> lerp(RtTransform<R, T> t1, RtTransform<R, T> t2, real64 f){
	return RtTransform<R, T>(	lerp(t1.rotation, t2.rotation, f),
								lerp(t1.translation, t2.translation, f));
}

template <typename S, typename R, typename T>
SrtTransform<S, R, T> lerp(SrtTransform<S, R, T> t1, SrtTransform<S, R, T> t2, real64 f){
	return SrtTransform<S, R, T>(	lerp(t1.scale, t2.scale, f),
									lerp(t1.rotation, t2.rotation, f),
									lerp(t1.translation, t2.translation, f));
}

/// SrtTransform commonReplaced by RtTransform
template <typename S1, typename R1, typename T1, typename S2, typename R2>
struct CommonReplaced<SrtTransform<S1, R1, T1>, RtTransform<S2, R2>> {
	using Dst= SrtTransform<S1, R1, T1>;
	using Src= RtTransform<S2, R2>;
	
	static Dst value(Dst dst, Src src){
		return 	Dst(dst.scale,
					commonReplaced(dst.rotation, src.rotation),
					commonReplaced(dst.translation, src.translation)
				);
	}
};

/// RtTransform commonReplaced by SrtTransform
template <typename S1, typename R1, typename S2, typename R2, typename T2>
struct CommonReplaced<RtTransform<S1, R1>, SrtTransform<S2, R2, T2>> {
	using Dst= RtTransform<S1, R1>;
	using Src= SrtTransform<S2, R2, T2>;
	
	static Dst value(Dst dst, Src src){
		return 	Dst(commonReplaced(dst.rotation, src.rotation),
					commonReplaced(dst.translation, src.translation)
				);
	}
};

/// SrtTransform commonReplaced by different type of SrtTransform
template <typename S1, typename R1, typename T1, typename S2, typename R2, typename T2>
struct CommonReplaced<SrtTransform<S1, R1, T1>, SrtTransform<S2, R2, T2>> {
	using Dst= SrtTransform<S1, R1, T1>;
	using Src= SrtTransform<S2, R2, T2>;

	static Dst value(Dst dst, Src src){
		return	Dst(commonReplaced(dst.scale, src.scale),
					commonReplaced(dst.rotation, src.rotation),
					commonReplaced(dst.translation, src.translation)
				);
	}
};

template <typename R, typename T>
struct IsFinite<RtTransform<R, T>> {
	static bool value(const RtTransform<R, T>& v){
		return isFinite(v.rotation) && isFinite(v.translation);
	}
};

template <typename S, typename R, typename T>
struct IsFinite<SrtTransform<S, R, T>> {
	static bool value(const SrtTransform<S, R, T>& v){
		return	isFinite(v.scale) &&
				isFinite(v.rotation) &&
				isFinite(v.translation);
	}
};

template <typename T>
struct Hash32<TTransform<T>> {
	uint32 operator()(const TTransform<T>& t) const {
		return util::hash32(t.translation);
	}
};

template <typename R, typename T>
struct Hash32<RtTransform<R, T>> {
	uint32 operator()(const RtTransform<R, T>& t) const {
		std::array<uint32, 2> arr= { util::hash32(t.rotation), util::hash32(t.translation) };
		return rawArrayHash32(arr.data(), arr.size());
	}
};

template <typename S, typename R, typename T>
struct Hash32<SrtTransform<S, R, T>> {
	uint32 operator()(const SrtTransform<S, R, T>& t) const {
		std::array<uint32, 3> arr= { util::hash32(t.scale), util::hash32(t.rotation), util::hash32(t.translation) };
		return rawArrayHash32(arr.data(), arr.size());
	}
};

} // util
} // clover

#endif // CLOVER_UTIL_TRANSFORM_HPP