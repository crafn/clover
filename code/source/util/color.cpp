#include "color.hpp"

namespace clover {
namespace util {

const Color::PtrArray Color::_v = { &Color::r, &Color::g, &Color::b, &Color::a };

Color Color::hsl(util::Vec3f hsl){
	Color c;
	c.setHsl(hsl);
	return c;
}

Color::Color(){
	r= g= b= a= 1.0f;
}

Color::Color(const std::initializer_list<real32>& l){
	std::copy(l.begin(), l.end(), &v(0));
}

Color::Color(const Color& other){
	std::copy(&other[0], &other[0]+size(), &v(0));
}

Color::Color(const Vec4f& value){
	for(uint32 i=0; i<4; ++i) v(i)= value[i];
}

Color::Color(const real32& value){
	for(uint32 i=0; i<4; ++i) v(i)= value;
}

Color& Color::operator=(const Color& other){
	std::copy(&other[0], &other[0]+size(), &v(0));
	return *this;
}

void Color::setHsl(const util::Vec3f& hsl){
	real32 h= hsl[0];
	real32 s= hsl[1];
	real32 l= hsl[2];
	
	if (s < util::epsilonf){
		r= g= b= l; // Achromatic
	} else {
		auto hueToRgb= [] (real32 p, real32 q, real32 t) -> real32 {
			if (t < 0) t += 1;
			if (t > 1) t -= 1;
			if (t < 1.0/6.0) return p + (q-p)*6*t;
			if (t < 1.0/2.0) return q;
			if (t < 2.0/3.0) return p + (q-p)*(2.0/3.0 - t)*6;
			return p;
		};
	
		real32 q= l < 0.5 ? l*(l+s) : l + s - l*s;
		real32 p= 2*l - q;
		r= hueToRgb(p, q, h + 1.0/3.0);
		g= hueToRgb(p, q, h);
		b= hueToRgb(p, q, h - 1.0/3.0);
	}
}

util::Vec3f Color::getHsl() const {
	
	real32 max= std::max(std::max(r,g),b);
	real32 min= std::min(std::min(r,g),b);
	
	util::Vec3f ret;
	ret[2]= (max+min)*0.5;
	
	if (max == min){
		ret[0]= ret[1]= 0;
	}
	else {
		real32 d= max-min;
		ret[1]= ret[2] > 0.5 ? d / (2 - max - min) : d / (max + min);
		if (r > g && r > b){
			ret[0]= (g - b)/d + (g < b ? 6 : 0);
		}
		else if (g > b){
			ret[0]= (g - r)/d + 2;
		}
		else {
			ret[0]= (r - g)/d + 4;
		}
		ret[0]/= ret[0];
	}
	
	return ret;
}

void Color::setHsv(const util::Vec3f& hsv){
	
	int32 i= std::floor(hsv[0]*6);
	real32 f= hsv[0]*6 - i;
	real32 p= hsv[2]*(1 - hsv[1]);
	real32 q= hsv[2]*(1- f*hsv[1]);
	real32 t= hsv[2]*(1 - (1 - f)*hsv[1]);

	switch(i % 6){
		case 0: r= hsv[2], g= t, b= p; break;
        case 1: r= q, g= hsv[2], b= p; break;
        case 2: r= p, g= hsv[2], b= t; break;
        case 3: r= p, g= q, b= hsv[2]; break;
        case 4: r= t, g= p, b= hsv[2]; break;
        case 5: r= hsv[2], g= p, b= q; break;
	}
	
}

util::Vec3f Color::getHsv() const {
	real32 max= std::max(std::max(r, g), b);
	real32 min= std::min(std::min(r, g), b);
	
	util::Vec3f ret;
	ret[2]= max;
	
	real32 d= max - min;
	ret[1]= max < util::epsilonf ? 0 : d / max;
	
	if (d < util::epsilonf){
		ret[0]= 0; // Achromatic
	}
	else {
		if (r > g && r > b){
			ret[0]= (g - b)/d + (g < b ? 6 : 0);
		}
		else if (g > b){
			ret[0]= (b - r)/d + 2;
		}
		else {
			ret[0]= (r - g)/d + 4;
		}
		
		ret[0] /= 6.0;
	}
	
	return ret;
}

Color lerp(Color c1, Color c2, real64 t){
	return lerp(c1.asVec(), c2.asVec(), t);
}

} // util
} // clover
