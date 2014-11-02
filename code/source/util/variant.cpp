#include "variant.hpp"

namespace clover {
namespace util {

Variant::Variant():
			type((Type)-1),
			handle(0){
	value.uInt=0;
}

Variant& Variant::operator=(real32 a){
	type= Type_Real32;

	value.uFloat= a;

	return *this;
}

Variant& Variant::operator=(real64 a){
	type= Type_Real64;
	value.uDouble= a;

	return *this;
}

Variant& Variant::operator=(int32 a){
	type= Type_Int32;
	value.uInt= a;

	return *this;
}

Variant& Variant::operator=(const util::Str8& a){
	type= Type_String;
	str= a;

	return *this;
}

Variant& Variant::operator=(const game::WeHandle& a){
	type= Type_Handle;
	handle= a;

	return *this;
}

Variant& Variant::operator=(const util::Vec2d& vec){
	type= Type_Vec2f;
	vector2f= vec;

	return *this;
}

Variant& Variant::operator=(const util::Vec2i& vec){
	type= Type_Vec2i;
	vector2i= vec;

	return *this;
}

real32		 Variant::getF(){
	if (type != Type_Real32 )
		throw Exception("Variant::getF(): type is %i", type);

	return value.uFloat;
}

real64		Variant::getD(){
	if (type != Type_Real64)
		throw Exception("Variant::getD(): type is %i", type);

	return value.uDouble;
}


int32		  Variant::getI(){
	if (type != Type_Int32)
		throw Exception("Variant::getI(): type is %i", type);

	return value.uInt;
}

util::Vec2d		  Variant::getVec2d(){
	if (type != Type_Vec2f)
		throw Exception("Variant::getutil::Vec2d(): type is %i", type);

	return vector2f;
}
util::Vec2i		  Variant::getVec2i(){
	if (type != Type_Vec2i)
		throw Exception("Variant::getutil::Vec2i(): type is %i", type);

	return vector2i;
}

util::Str8 Variant::getStr(){
	if (type != Type_String) throw Exception("Variant::getF(): type is %i", type);

	return str;

}

game::WeHandle	  Variant::getHandle(){
	if (type != Type_Handle) throw Exception("Variant::getF(): type is %i", type);
	return handle;
}

} // util
} // clover