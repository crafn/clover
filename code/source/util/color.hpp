#ifndef CLOVER_UTIL_COLOR_HPP
#define CLOVER_UTIL_COLOR_HPP

#include "build.hpp"
#include "util/hash.hpp"
#include "util/vector.hpp"

namespace clover {
namespace util {

class Color {
public:
	static Color hsl(util::Vec3f hsl);

	/// White by default (all components == 1.0f)
	Color();
	Color(const std::initializer_list<real32>& l);
	Color(const Color&);
	Color(const Vec4f&);
	Color(const real32& value);
	
	Color& operator=(const Color& other);
	
	static constexpr SizeType size(){ return 4; }
	real32& operator[](SizeType i){ return v(i); }
	const real32& operator[](SizeType i) const { return v(i); }
	
	Color operator*(real32 f) const { return Color{r*f, g*f, b*f, a*f}; }
	Color operator*(const Color& c) const { return Color{r*c.r, g*c.g, b*c.b, a*c.a}; }
	Color operator+(const Color& c) const { return Color{r+c.r, g+c.g, b+c.b, a+c.a}; }
	
	Color& operator*=(real32 f){ return *this= *this * f; }
	Color& operator*=(const Color& c){ return *this= *this * c; }
	Color& operator+=(const Color& c){ return *this= *this + c; }
	
	bool operator==(const Color& c) const { return r == c.r && g == c.g && b == c.b && a == c.a; }
	
	// Hue, saturation, lightness
	void setHsl(const util::Vec3f& hsl);
	util::Vec3f getHsl() const;
	
	// Hue, saturation, value
	void setHsv(const util::Vec3f& hsv);
	util::Vec3f getHsv() const;
	
	util::Vec4f asVec() const { return util::Vec4f{r, g, b, a}; }

	real32 r;
	real32 g;
	real32 b;
	real32 a;
	
private:
	typedef real32 Color::* const PtrArray[4];
	static const PtrArray _v;
	
	inline
	real32& v(SizeType i) {
		return this->*_v[i];
	}
	
	inline
	const real32& v(SizeType i) const{
		return this->*_v[i];
	}
};

template <>
struct Hash32<Color> {
	uint32 operator()(const Color& c) const {
		return rawArrayHash32(&c[0], 4);
	}
};

Color lerp(Color c1, Color c2, real64 t);

} // util
} // clover

#endif // CLOVER_UTIL_COLOR_HPP
