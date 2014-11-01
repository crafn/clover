#ifndef CLOVER_UTIL_VARIANT_HPP
#define CLOVER_UTIL_VARIANT_HPP

#include <string>

#include "util/math.hpp"
#include "util/string.hpp"
#include "game/worldentity_handle.hpp" /// @todo Fix: this is bad in util

namespace clover {
namespace util {

/// @todo Uncrappify
class Variant {
public:
    enum Type {
        Type_Int32,
        Type_Real32,
        Type_Real64,
        Type_String,
        Type_Handle, // Handle to WE
        Type_Vec2f,
        Type_Vec2i,
		Type_Ptr
    };

    Variant();

    Variant& operator=(real32);
    Variant& operator=(real64);
    Variant& operator=(int);
    Variant& operator=(const util::Str8&);
    Variant& operator=(const game::WeHandle&);
    Variant& operator=(const util::Vec2d&);
    Variant& operator=(const util::Vec2i&);
	
	template<typename T>
	Variant& operator=(T* t){
		type= Type_Ptr;
		value.uPtr= (void*)t;
		return *this;
	}

    real32      getF();
    real64      getD();
    int32		getI();
    util::Str8 		getStr();
    game::WeHandle    getHandle();
    util::Vec2d       getVec2d();
    util::Vec2i       getVec2i();

	template<typename T>
	T* getPtr(){
		ensure(type == Type_Ptr);
		ensure(value.uPtr);
		T* ret= reinterpret_cast<T*>(value.uPtr);
		ensure(ret);
		return ret;
	}

private:
    Type type;

    game::WeHandle handle;
    util::Str8 str;
    util::Vec2d vector2f;
    util::Vec2i vector2i;

    union Value {
        real32 uFloat;
        real64 uDouble;
        int32 uInt;
		void* uPtr;
    };

    Value value;
};

} // util
} // clover

#endif // CLOVER_UTIL_VARIANT_HPP