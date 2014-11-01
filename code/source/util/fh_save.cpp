#include "fh_save.hpp"
#include "util/dyn_array.hpp"
#include "debug/debugprint.hpp"
#include "game/worldentity_handle.hpp"

namespace clover {
namespace util {

const SaveFieldHolder& SaveFieldHolder::operator>>(util::RawPack& pack) const {

	for (uint32 i=0; i<fields.size(); ++i){
		// Fomat: typeid<uint8>, namehash<uint32>, data count<uint16>
		uint32 dataCount=1;
		if (fields[i].isArray()){

#define TYPE(name_, type_) \
		case Field::name_: dataCount= static_cast<util::DynArray<type_>*>(fields[i].getPtr())->size(); break;

			switch(fields[i].getType()){
				#include "fh_types.def"
				default: ensure(0);
			}
#undef TYPE

			if ((int32)dataCount > fields[i].getMaxCount()){
				dataCount= fields[i].getMaxCount();
			}
		}

		pack << (uint8)fields[i].getType() << (uint32)fields[i].getHash() << (uint16)dataCount ;

		if (!fields[i].isArray()){

#define TYPE(name_, type_)															\
			case Field::name_:														\
				/*print(debug::Ch::General, debug::Vb::Trivial,*/ 							\
				/*	"write index: %i", pack.size());*/								\
					pack << *static_cast<type_ *>(fields[i].getPtr());				\
			break;


		switch (fields[i].getType()){
			#include "fh_types.def"
			
			default: release_ensure(0 );
		}

#undef TYPE

		}
		else {
		
			//
			// util::DynArray
			//

			uint32 len= fields[i].getMaxCount();
			if (dataCount < len) len= dataCount;

			//print(debug::Ch::Save, debug::Vb::Trivial, "Save: dataCount: %i len: %i", dataCount, len);

#define TYPE(name_, type_) \
			case Field::name_:{														\
				/*print(debug::Ch::General, debug::Vb::Trivial,*/ 							\
				/*	"write index: %i", pack.size());*/								\
				util::DynArray<type_>& vec= 												\
					(*static_cast<util::DynArray<type_> *>(fields[i].getPtr())); 			\
				/*if (vec.size() != len) vec.resize(len);	//Turha?*/				\
				for (uint32 a=0; a<len; ++a)										\
					pack << vec[a];													\
			}break;
		switch (fields[i].getType()){
			#include "fh_types.def"

			default: release_ensure(0);
		}

#undef TYPE
		}
	}
	return *this;
}

SaveFieldHolder& SaveFieldHolder::operator<<(util::RawPack& pack){
	//print(debug::Ch::General, debug::Vb::Trivial, "pack size: %i", pack.size());
	SizeType found_count= 0;
	while(!pack.eof() && found_count < fields.size()){
		uint8 type;
		uint16 arrayLen;
		uint32 hash;

		pack >> type >> hash >> arrayLen;

		bool found=false;

		for (uint32 i=0; i<fields.size(); ++i){
			//print(debug::Ch::General, debug::Vb::Trivial, "test %i %i %i : %i %i", fields[i].getType() == type,fields[i].getMaxCount() == arrayLen,fields[i].getHash() == hash, fields[i].getHash(), hash);
			if (fields[i].getType() == type &&
				fields[i].getHash() == hash){

				++found_count;

				found= true;
				if (!fields[i].isArray()){

#define TYPE(name_, type_)																\
					case Field::name_:{													\
																						\
						if (fields[i].hasChangeFunc()){									\
							/*print(debug::Ch::General, debug::Vb::Trivial, "read index: %i", pack.tellg()); */ \
							type_ temp;													\
							pack >>	temp;												\
																						\
																						\
							fields[i].name_ ## ChangeFunc(temp);						\
																						\
						}																\
						else {															\
								pack >> *static_cast<type_ *>(fields[i].getPtr());		\
						}} break;


				switch (fields[i].getType()){
					#include "fh_types.def"
					break;
					default: release_ensure(0);
				}

#undef TYPE
				}
				else {

					//
					// Taulukko (util::DynArray)
					//

					uint32 len=fields[i].getMaxCount();
					if (arrayLen < len) len= arrayLen;

					//print(debug::Ch::Save, debug::Vb::Trivial, "Load: arrayLen: %i len: %i", arrayLen, len);

#define TYPE(name_, type_)																\
					case Field::name_:{													\
																						\
						if (fields[i].hasChangeFunc()){									\
							/* Kutsutaan changeFunccia kun */							\
							/* sellainen on kerran asetettu */							\
							/*print(debug::Ch::General, debug::Vb::Trivial, "read index: %i", pack.tellg()); */ \
							util::DynArray<type_> temp;									\
							if (temp.size() != len)temp.resize(len);					\
							for (uint32 a=0; a<len; ++a){								\
								type_ t;												\
								pack >>	t;												\
								temp[a]= t;												\
							}															\
																						\
							fields[i].name_ ## ArrayChangeFunc(temp);					\
																						\
						}																\
						else {															\
						  /* Luetaan suoraan muuttujaan */								\
							util::DynArray<type_>& vec= 										\
								(*static_cast<util::DynArray<type_> *>(fields[i].getPtr())); \
							/*vec.clear();*/											\
							if (vec.size() != len)vec.resize(len);						\
							for (uint32 a=0; a<len; ++a){								\
																						\
								pack >> vec[a];											\
																						\
							}															\
						}} break;


				switch (fields[i].getType()){
					#include "fh_types.def"


					default: release_ensure(0); //Määrittelemätön tyyppi
				}

#undef TYPE

				}
			}
		}
		if (!found){
			print(debug::Ch::General, debug::Vb::Critical, "Cannot find: %i, read index: %i, count: %i, hash: %i", type, pack.tellg(), arrayLen, hash);
			for (uint32 i=0; i<fields.size(); ++i)
				print(debug::Ch::General, debug::Vb::Trivial, "%i %i %i", fields[i].getType(), fields[i].getMaxCount(), fields[i].getHash());
			//ensure(0);
		}
	}

	return *this;
}

} // util
} // clover