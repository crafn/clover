#include "fieldholder.hpp"
#include "hash.hpp"
#include "debug/debugprint.hpp"

namespace clover {
namespace util {

FieldHolder::Field::Field(void * ptr_, Type type_, const util::Str8 name, int32 len/*, uint8 sub_type*/):
				ptr(ptr_),
				type(type_),
				/*subType(sub_type),*/
				arrayLen(len),
				changeFuncSet(false){
	hash= util::hash32(name);
}

FieldHolder::FieldHolder():weHandleStored(false){
}

#define TYPE(name_, type_)																		\
	FieldHolder::Field* FieldHolder::add(type_ & ref, const util::Str8 &name/*, uint8 subtype*/){		\
			fields.pushBack(Field(&ref, Field:: name_, name, -1/*, subtype*/));					\
			for (int32 i=0; i<(int32)fields.size()-1; ++i){										\
																								\
				ensure(fields[i].getHash() != fields.back().getHash());							\
			}																					\
			return &fields.back();																\
		}																						\
																								\
	FieldHolder::Field* FieldHolder::add(	util::DynArray<type_> & ref,								\
											const util::Str8 &name,										\
											uint32 max_count ){									\
			fields.pushBack(Field(&ref, Field:: name_, name, max_count/*, subtype*/));			\
			for (int32 i=0; i<(int32)fields.size()-1; ++i){										\
																								\
				ensure(fields[i].getHash() != fields.back().getHash());							\
			}																					\
			return &fields.back();																\
		}

#include "fh_types.def"

#undef TYPE

FieldHolder::Field* FieldHolder::getFieldByName(const util::Str8& name){
	uint32 hash= Hash32<util::Str8>()(name);
	for (uint32 i=0; i<fields.size(); ++i){
		if (hash == fields[i].getHash()){
			return &fields[i];
		}
	}
	return 0;
}

} // util
} // clover