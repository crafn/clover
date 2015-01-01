#ifndef CLOVER_UTIL_COORD_HPP
#define CLOVER_UTIL_COORD_HPP

#include "build.hpp"
#include "math.hpp"

namespace clover {
namespace util {

/// Point in specific coordinate system
/// @todo Move to gui/ui
/// @todo Remove when refactoring gui
struct Coord {
public:
	enum Type {
		None,
		World,	  // World coordinate
		View_Scale, // (-1, -1) -> (1,1) fills whole screen (but doesn't fit)
		View_Fit,	// (-1, -1) -> (1,1) will always fit on screen (but leaves empty areas depending on aspect ratio)
		View_Stretch,  // (-1, -1) -> (1,1) fills whole screen but doesn't preserve aspect ratio
		View_Pixels
	};

	Coord(real64 d, Type t= World);
	Coord(RealVector<double, 2> d, Type t= World, bool relative=false);
	Coord(Type t= World);
	Coord(const Coord& other);
	Coord(Coord&&);

	Coord& operator=(const Coord&);
	Coord& operator=(Coord&&);
	
	/// View_Stretch
	static Coord VSt(real64 d);
	static Coord VSt(RealVector<double, 2> d);
	
	/// View_Scale
	static Coord VSc(real64 d);
	static Coord VSc(RealVector<double, 2> d);
	
	/// View_Fit
	static Coord VF(real64 d);
	static Coord VF(RealVector<double, 2> d);
	
	/// Pixels
	static Coord P(real64 d);
	static Coord P(RealVector<double, 2> d);
	
	/// World
	static Coord W(real64 d);
	static Coord W(RealVector<double, 2> d);

	Coord operator+(const Coord& other) const;
	Coord operator-(const Coord& other) const; // Makes relative!
	
	Coord operator*(const util::Vec2d& vec) const;
	Coord operator*(const Coord& other) const;
	Coord operator*(const real64 mul) const;

	/// @brief util::Set value
	void setValue(const util::Vec2d& vec);

	/// @brief Convert other.value to Coord's type and set
	Coord& operator%=(const Coord& other);

	Coord& operator+=(const Coord& other);
	Coord& operator+=(const util::Vec2d& vec);
	Coord& operator-=(const Coord& other);
	Coord& operator-=(const util::Vec2d& vec);
	
	Coord& operator*=(const util::Vec2d& vec);
	Coord& operator*=(const Coord& other);
	Coord& operator*=(const real64 mul);
	
	void setRelative(bool rel=true){ relative= rel; }
	bool isRelative() const { return relative; }

	void convertTo(Type t);
	Coord converted(Type t) const;

	template <typename T>
	Coord rotated(const T& r) const { return Coord(value.rotated(r), type, relative); }

	util::Vec2d& getValue(){ return value; }
	const util::Vec2d& getValue() const { return value; }
	Type getType() const { return type; }

	const real64& operator[](uint32 i) const { return value[i]; }
	real64& operator[](uint32 i) { return value[i]; }
	
	void setType(Type t){ type= t; }

	Coord abs() const;

	Coord onlyY() const;
	Coord onlyX() const;

	real64& x, &y; // These are needed so util::BoundingBox<Coord> work (traits would be an alternative option)

private:
	Coord getConverted(const Coord& other) const;
	
	Type type;
	util::Vec2d value;
	
	bool relative;
};

} // util
} // clover

#endif // CLOVER_UTIL_COORD_HPP
