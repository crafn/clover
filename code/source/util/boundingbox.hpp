#ifndef CLOVER_UTIL_BOUNDINGBOX_HPP
#define CLOVER_UTIL_BOUNDINGBOX_HPP

#include "build.hpp"
#include "vector.hpp"
#include "coord.hpp"
#include "traits.hpp"

namespace clover {
namespace util {
namespace detail {

/// T should be integer or IntegerVector
template <typename T>
struct BbEpsilon {
	static T eval() { return T(1); }
};

template <>
struct BbEpsilon<float> {
	static double eval() { return std::numeric_limits<float>::epsilon(); }
};

template <>
struct BbEpsilon<double> {
	static double eval() { return std::numeric_limits<double>::epsilon(); }
};

template <typename T, SizeType N>
struct BbEpsilon<RealVector<T, N>> {
	static RealVector<T, N> eval()
	{ return RealVector<T, N>(std::numeric_limits<T>::epsilon()); }
};

template <typename T>
struct Component {
	static constexpr SizeType count= 1;
	static T& get(T& t, SizeType i) { return t; }
	static const T& get(const T& t, SizeType i) { return t; }
};

template <typename C, SizeType N>
struct Component<RealVector<C, N>> {
	using T= RealVector<C, N>;
	static constexpr SizeType count= N;
	static C& get(T& t, SizeType i) { return t[i]; }
	static const C& get(const T& t, SizeType i) { return t[i]; }
};

template <typename C, SizeType N>
struct Component<IntegerVector<C, N>> {
	using T= IntegerVector<C, N>;
	static constexpr SizeType count= N;
	static C& get(T& t, SizeType i) { return t[i]; }
	static const C& get(const T& t, SizeType i) { return t[i]; }
};

/// @todo Could be made public
template <typename T>
auto component(T&& t, SizeType i)
-> decltype(Component<Plain<T>>::get(std::forward<T>(t), i))
{	 return Component<Plain<T>>::get(std::forward<T>(t), i); }

template <typename T>
constexpr SizeType componentCount()
{ return Component<T>::count; }

} // detail

template <typename T>
class BoundingBox {
public:
	using Point= T;
	using This= BoundingBox<T>;

	static T epsilon() { return detail::BbEpsilon<T>::eval(); }

	static BoundingBox zero(){ return BoundingBox(T(0), T(0)); }

	BoundingBox();
	BoundingBox(BoundingBox&& b)= default;
	BoundingBox(const BoundingBox& b)= default;
	BoundingBox(const T& min, const T& max);

	BoundingBox& operator=(const BoundingBox&)= default;
	BoundingBox& operator=(BoundingBox&&)= default;

	/// @brief Moves bounding box
	BoundingBox operator+(const T& v) const;

	/// @brief Appends bounding box to contain a point
	void append(const T& pos);
	void append(const This& other);

	/// @brief Resets BoundingBox
	void reset();

	bool isSet() const;

	/// Inclusive minimum
	const T& getMin() const { return min; }
	/// Inclusive maximum
	const T& getMax() const { return max; }

	T getCenter() const { return getMin() + getRadius(); }
	T getRadius() const { return getSize()*0.5; }
	T getSize() const { ensure(isSet()); return max-min; }
	T getInclusiveSize() const { return getSize() + epsilon(); }
	
	template <typename U>
	U casted() const { return U(min.casted<typename U::Point>(), max.casted<typename U::Point>()); }

private:
	T min;
	T max;
};

#include "boundingbox.tpp"

} // util
} // clover

#endif // CLOVER_UTIL_BOUNDINGBOX_HPP
