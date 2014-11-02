#include "coord.hpp"
#include "hardware/device.hpp"
#include "script/script_mgr.hpp"
#include "util/profiling.hpp"
#include "visual/camera.hpp"
#include "visual/visual_mgr.hpp"
#include "visual/camera_mgr.hpp"

namespace clover {
namespace util {

util::Vec2d viewStretchToWorldVec(util::Vec2d view){
	return Coord::VSt(view).converted(Coord::World).getValue();
}

void Coord::registerToScript(){
	script::gScriptMgr->registerGlobalFunction(viewStretchToWorldVec, "viewStretchToWorldVec");
}

Coord::Coord(real64 d, Type t):
					x(value.x),
					y(value.y),
					type(t),
					value(d),
					relative(false){

}

Coord::Coord(RealVector<double, 2> d, Type t, bool relative_):
					x(value.x),
					y(value.y),
					type(t),
					value(d),
					relative(relative_){
	
}

Coord::Coord(Type t):
		x(value.x),
		y(value.y),
		type(t),
		value(0),
		relative(false){
}

Coord::Coord(const Coord& other):
		x(value.x),
		y(value.y),
		type(other.type),
		value(other.value),
		relative(other.relative){
	
}

Coord::Coord(Coord&& other):
		x(value.x),
		y(value.y),
		type(other.type),
		value(other.value),
		relative(other.relative){

}

Coord& Coord::operator=(const Coord& other){
	type= other.type;
	value= other.value;
	relative= other.relative;

	return *this;
}

Coord& Coord::operator=(Coord&& other){
	type= other.type;
	value= other.value;
	relative= other.relative;

	return *this;
}

Coord Coord::VSt(real64 d){
	Coord c(d, View_Stretch);
	return c;
}

Coord Coord::VSt(RealVector<double, 2> d){
	Coord c(d, View_Stretch);
	return c;
}

Coord Coord::VSc(real64 d){
	Coord c(d, View_Scale);
	return c;
}

Coord Coord::VSc(RealVector<double, 2> d){
	Coord c(d, View_Scale);
	return c;
}

Coord Coord::VF(real64 d){
	Coord c(d, View_Fit);
	return c;
}

Coord Coord::VF(RealVector<double, 2> d){
	Coord c(d, View_Fit);
	return c;
}

Coord Coord::P(real64 d){
	Coord c(d, View_Pixels);
	return c;
}

Coord Coord::P(RealVector<double, 2> d){
	Coord c(d, View_Pixels);
	return c;
}

Coord Coord::W(real64 d){
	Coord c(d, World);
	return c;
}

Coord Coord::W(RealVector<double, 2> d){
	Coord c(d, World);
	return c;
}

void Coord::convertTo(Type t){
	PROFILE();

	ensure(std::isfinite(value.x) && std::isfinite(value.y));
	ensure(t != None);
	if (t == type) return;
	
	util::Vec2d campos;
	real32 camscale=1;
	// Radius of viewport in world coordinates
	util::Vec2d view_world_rad;
	
	if (type == World || t == World){
		 visual::Camera& cam= visual::gVisualMgr->getCameraMgr().getSelectedCamera();
		 campos= cam.getPosition();
		 camscale= cam.getScale();
		 ensure(camscale > 0);
		 view_world_rad= util::Vec2d(1.0)/camscale;
	}
	
	view_world_rad *= hardware::gDevice->getAspectVector().inversed();
	
	util::Vec2d viewport_pixel_rad= (util::Vec2d)hardware::gDevice->getViewportSize()*0.5;

	util::Vec2d aspect_vector= hardware::gDevice->getAspectVector();
	real64 aspect_component_dif= util::abs(aspect_vector.x-aspect_vector.y);
	
	util::Vec2d view_stretch_pos= value;
	
	if (type == World){
		if (!relative)
			view_stretch_pos -= campos;
	
		view_stretch_pos *= view_world_rad.inversed();
	}
	else if (type == View_Scale){
		view_stretch_pos *= aspect_vector;
	}
	else if (type == View_Fit){
		view_stretch_pos /= (aspect_component_dif + 1);
		view_stretch_pos *= aspect_vector;
	}
	else if (type == View_Pixels){
		view_stretch_pos *= viewport_pixel_rad.inversed();
	}
	
	// Now view_strech_pos is in View_Stretch coordinates
	
	util::Vec2d converted= view_stretch_pos;
	if (t == World){
		converted *= view_world_rad;
		if (!relative)
			converted += campos;
	}
	else if (t == View_Scale){
		converted *= aspect_vector.inversed();
	}
	else if (t == View_Fit){
		converted *= aspect_vector.inversed();
		converted *= (aspect_component_dif + 1);
	}
	else if (t == View_Pixels){
		converted *= viewport_pixel_rad;
	}

	ensure(std::isfinite(converted.x) && std::isfinite(converted.y));

	value= converted;
	type= t;
}

Coord Coord::converted(Type t) const {
	Coord c= *this;
	c.convertTo(t);
	return c;
}

Coord Coord::operator+(const Coord& other) const {
	Coord c(type);
	c.value= value + getConverted(other).value;
	return c;
}

Coord Coord::operator-(const Coord& other) const {
	Coord c(*this);
	c.value= value - getConverted(other).value;
	// Added this because it might be true for all cases, and util::BoundingBox getSize() must return relative value
	// If a problems occurs, solve it with traits in getSize() or something instead of forcing coord to be relative here
	c.relative= true;
	return c;
}

Coord Coord::operator*(const util::Vec2d& vec) const {
	Coord c(*this);
	c.value= value*vec;
	return c;
}

Coord Coord::operator*(const Coord& other) const {
	Coord c(*this);
	c.value= value*getConverted(other).value;
	return c;
}

Coord Coord::operator*(const real64 mul) const {
	Coord c(*this);
	c.value= value*mul;
	return c;
}

void Coord::setValue(const util::Vec2d& vec) {
	value= vec;
}

Coord& Coord::operator%=(const Coord& other){
	value= getConverted(other).value;
	return *this;
}

Coord& Coord::operator+=(const Coord& other) {
	return *this = *this + other;
}

Coord& Coord::operator+=(const util::Vec2d& vec) {
	value += vec;
	return *this;
}

Coord& Coord::operator-=(const Coord& other) {
	return *this = *this - other;
}

Coord& Coord::operator-=(const util::Vec2d& vec) {
	value -= vec;
	return *this;
}

Coord& Coord::operator*=(const util::Vec2d& vec) {
	return *this = *this * vec;
}

Coord& Coord::operator*=(const Coord& other){
	return *this = *this * other;
}

Coord& Coord::operator*=(const real64 mul) {
	return *this = *this * mul;
}

Coord Coord::getConverted(const Coord& other) const {
	if (other.type == type) return other;
	Coord c= other;
	c.convertTo(type);
	return c;
}

Coord Coord::abs() const {
	return (Coord(value.abs(), type));
}

Coord Coord::onlyY() const {
	return Coord({0,y}, type, relative);
}

Coord Coord::onlyX() const {
	return Coord({x,0}, type, relative);
}

} // util
} // clover