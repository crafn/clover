#ifndef CLOVER_UTIL_PACK_HPP
#define CLOVER_UTIL_PACK_HPP

#include "build.hpp"
#include "dyn_array.hpp"
#include "global/exception.hpp"
#include "math.hpp"
#include "string.hpp"

#include <stddef.h>

namespace clover {
namespace game {

/// @todo Shouldn't be in util >:(
class WeHandle;

} // game
namespace util {

/// Archive for binary data
/// @todo Should be replaced with util::RawArchive
class RawPack {
public:

    static size_t uint8ToBuf(void *buf, const uint8 d);
    static size_t uint8FromBuf(const void* buf, uint8& d);

    static size_t uint16ToBuf(void *buf, const uint16 d);
    static size_t uint16FromBuf(const void *buf, uint16& d);

    static size_t uint32ToBuf(void *buf, const uint32 d);
    static size_t uint32FromBuf(const void *buf, uint32& d);

	static size_t uint64ToBuf(void *buf, const uint64 d);
	static size_t uint64FromBuf(const void *buf, uint64& d);

    static size_t real32ToBuf(void *buf, const real32 d);
    static size_t real32FromBuf(const void *buf, real32& d);

    static size_t real64ToBuf(void *buf, const real64 d);
    static size_t real64FromBuf(const void *buf, real64& d);

	static size_t Vec2fToBuf(void *buf, const util::Vec2f& v);
	static size_t Vec2fFromBuf(const void *buf, util::Vec2f& v);

	static size_t Vec2dToBuf(void *buf, const util::Vec2d& v);
	static size_t Vec2dFromBuf(const void *buf, util::Vec2d& v);

	static size_t Vec2iToBuf(void *buf, const util::Vec2i& v);
	static size_t Vec2iFromBuf(const void *buf, util::Vec2i& v);

	static size_t boolToBuf(void *buf, const bool d);
	static size_t boolFromBuf(const void *buf, bool& d);

	RawPack(uint32 reserve=64);

	void add(const RawPack& p);
	RawPack& operator<<(const util::RawPack& p){ add(p); return *this; } // Kirjottaa aina loppuun, vaikka pitäis kirjottaa position kohdalle!


	void addData(const uint8 * buf, uint32 size);

#define PACK_OPERATOR(type_)  								\
		RawPack& operator<<(const type_& value);			\
		RawPack& operator>>(type_& value);

	PACK_OPERATOR(uint8)
	PACK_OPERATOR(uint16)
	PACK_OPERATOR(uint32)
	PACK_OPERATOR(uint64)
	PACK_OPERATOR(int8)
	PACK_OPERATOR(int16)
	PACK_OPERATOR(int32)
	PACK_OPERATOR(int64)
	PACK_OPERATOR(real32)
	PACK_OPERATOR(real64)
	PACK_OPERATOR(Vec2f)
	PACK_OPERATOR(Vec2d)
	PACK_OPERATOR(Vec2i)
	PACK_OPERATOR(bool)
	PACK_OPERATOR(game::WeHandle)
	PACK_OPERATOR(util::Str8)

#undef PACK_OPERATOR

#define SIZEOF(type_, size)				\
	static constexpr size_t sizeOf ## type_ = size;

	SIZEOF(uint8, 1)
	SIZEOF(uint16, 2)
	SIZEOF(uint32, 4)
	SIZEOF(uint64, 8)
	SIZEOF(int8, 1)
	SIZEOF(int16, 2)
	SIZEOF(int32, 4)
	SIZEOF(int64, 8)

	SIZEOF(real32, 4)
	SIZEOF(real64, 8)
	SIZEOF(Vec2f, 8)
	SIZEOF(Vec2d, 16)
	SIZEOF(Vec2i, 8)
	SIZEOF(bool, 1)

#undef SIZEOF

	uint32 size() const{
		return data.size();
	}

	void setReadMode(bool b=true){
		readMode= true;
	}

	void seekg(uint32 index){
		position= index;
	}

	uint32 tellg() const {
		return position;
	}

	bool eof(){
		if (position >= data.size())
			return true;
		return false;
	}

	void clear(){
		data.clear();
	}

	void reserve(uint32 s){
		data.reserve(s);
	}

	uint8& operator[](uint32 i){
		ensure(i < data.size());
		return data[i];
	}

	const uint8& operator[](uint32 i) const {
		ensure(i < data.size());
		return data[i];
	}

	util::Str8 getAsString() const {
		util::Str8 ret;
		for (auto& m : data){ ret << m; }
		return (ret);
	}

	const util::DynArray<uint8>& getData() const {
		return data;
	}

	util::DynArray<uint8>& getData(){
		return data;
	}

protected:
	bool readMode;
	uint32 position;
	util::DynArray<uint8> data;
};

} // util
} // clover

#endif // CLOVER_UTIL_PACK_HPP