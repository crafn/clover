#include "pack.hpp"
#include "exception.hpp"
#include "misc.hpp"
#include "rawarchive.hpp"
#include "debug/debugprint.hpp"
#include "game/worldentity_handle.hpp"
#include "util/string.hpp"

namespace clover {
namespace util {

RawPack::RawPack(uint32 reserve){
	data.reserve(reserve);
	position= 0;
	readMode= false;
}

void RawPack::add(const RawPack& p){
	if (p.size())
		addData(&(p.data[0]), p.size());
}

void RawPack::addData(const uint8* buf, uint32 s){
	uint32 offset= data.size();
	data.resize(data.size() + s);
	memcpy(&data[offset], buf, s);
	position= data.size();
}

#define PACK_OPERATOR(type_)										\
	RawPack& RawPack::operator<<(const type_ & value){				\
		uint8 buf[sizeof(type_)];									\
		size_t written= type_ ## ToBuf((void*)buf, value);			\
		debug_ensure(written <= sizeof(type_));						\
		for (uint32 i=0; i<written; ++i){							\
			if (position < data.size())								\
				data[position]= buf[i];								\
			else													\
				data.pushBack(buf[i]);								\
																	\
			position += 1;											\
		}															\
		return *this;												\
	}																\
	RawPack& RawPack::operator>>(type_ & value){											\
		ensure(!data.empty());																\
		ensure(data.size() - sizeOf ## type_ >= 0);											\
																							\
		if (!readMode){																		\
			size_t read= type_ ## FromBuf((void*)&data[data.size()-sizeOf ## type_], value); \
																							\
																							\
			for (uint32 i=0; i<sizeOf ## type_; ++i)										\
				data.popBack();																\
		}																					\
		else {																				\
			/* ReadMode */																	\
			size_t read= type_ ## FromBuf((void*)&data[position], value);					\
			position += read;																\
																							\
		}																					\
																							\
																							\
		return *this;																		\
	}

PACK_OPERATOR(uint8)
PACK_OPERATOR(uint16)
PACK_OPERATOR(uint32)
PACK_OPERATOR(uint64)
PACK_OPERATOR(real32)
PACK_OPERATOR(real64)
PACK_OPERATOR(Vec2f)
PACK_OPERATOR(Vec2d)
PACK_OPERATOR(Vec2i)
PACK_OPERATOR(bool)

#undef PACK_OPERATOR

#define PACK_OPERATOR_INT(type_)						\
	RawPack& RawPack::operator<<(const type_ & value){	\
		return operator<<( (u ## type_ ) value );		\
	}													\
	RawPack& RawPack::operator>>(type_ & value){	\
		return operator>>( (u ## type_ &) value );		\
	}

PACK_OPERATOR_INT(int8)
PACK_OPERATOR_INT(int16)
PACK_OPERATOR_INT(int32)
PACK_OPERATOR_INT(int64)
#undef PACK_OPERATOR_INT

RawPack& RawPack::operator<<(const game::WeHandle & value){
	return operator<<(value.getId());
}
RawPack& RawPack::operator>>(game::WeHandle & value){
	game::WorldEntityId temp;
	operator>>(temp);
	value.setId(temp);
	return *this;
}

RawPack& RawPack::operator<<(const util::Str8& value){
	for (SizeType i=0; i<value.sizeBytes(); ++i){
		operator<<(value.cStr()[i]);
	}
	uint32 null= 0;
	operator<<(null);
	return *this;
}

RawPack& RawPack::operator>>(util::Str8& value){
	uint32 ch= 0;
	while (1) {
		operator>>(ch);
		if (ch == 0) break;
		value += util::Str8::format("%c", ch);
	}
	
	return *this;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


size_t RawPack::uint8ToBuf(void *buf, const uint8 d){
	return RawArchive::serializePod((uint8*)buf, &d);
}

size_t RawPack::uint8FromBuf(const void* buf, uint8& d){
	return RawArchive::deserializePod((uint8*)buf, &d);
}

size_t RawPack::uint16ToBuf(void *buf, const uint16 d){
	return RawArchive::serializePod((uint8*)buf, &d);
}

size_t RawPack::uint16FromBuf(const void *buf, uint16& d){
	return RawArchive::deserializePod((uint8*)buf, &d);
}

size_t RawPack::uint32ToBuf(void *buf, const uint32 d){
	return RawArchive::serializePod((uint8*)buf, &d);
}

size_t RawPack::uint32FromBuf(const void *buf, uint32& d){
	return RawArchive::deserializePod((uint8*)buf,&d);
}

size_t RawPack::real32ToBuf(void *buf, const real32 d){
	return RawArchive::serializePod((uint8*)buf, &d);
}

size_t RawPack::real32FromBuf(const void *buf, real32& d){
	return RawArchive::deserializePod((uint8*)buf, &d);
}

size_t RawPack::uint64ToBuf(void *buf, const uint64 d){
	return RawArchive::serializePod((uint8*)buf, &d);
}

size_t RawPack::uint64FromBuf(const void *buf, uint64& d){
	return RawArchive::deserializePod((uint8*)buf, &d);
}

size_t RawPack::real64ToBuf(void *buf, real64 d){
	return RawArchive::serializePod((uint8*)buf, &d);
}

size_t RawPack::real64FromBuf(const void *buf, real64& d){
	return RawArchive::deserializePod((uint8*)buf, &d);
}

size_t RawPack::Vec2fToBuf(void *buf, const util::Vec2f& v){
	return RawArchive::serializePod((uint8*)buf, &v.x, 2);
}

size_t RawPack::Vec2fFromBuf(const void *buf, util::Vec2f& v){
	return RawArchive::deserializePod((uint8*)buf, &v.x, 2);
}

size_t RawPack::Vec2dToBuf(void *buf, const util::Vec2d& v){
	return RawArchive::serializePod((uint8*)buf, &v.x, 2);
}

size_t RawPack::Vec2dFromBuf(const void *buf, util::Vec2d& v){
	return RawArchive::deserializePod((uint8*)buf, &v.x, 2);
}

size_t RawPack::Vec2iToBuf(void *buf, const util::Vec2i& v){
	return RawArchive::serializePod((uint8*)buf, &v.x, 2);
}

size_t RawPack::Vec2iFromBuf(const void *buf, util::Vec2i& v){
	return RawArchive::deserializePod((uint8*)buf, &v.x, 2);
}

size_t RawPack::boolToBuf(void *buf, const bool d){
	uint8 u=0;
	if (d)u= 1;
	return RawArchive::serializePod((uint8*)buf, &d);
}

size_t RawPack::boolFromBuf(const void *buf, bool& d){
	uint8 u;
	size_t size= RawArchive::deserializePod((uint8*)buf, &u);
	d= u;
	return size;
}

} // util
} // clover