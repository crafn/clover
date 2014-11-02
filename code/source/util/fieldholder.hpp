#ifndef UTIL_CLOVER_FIELDHOLDER_HPP
#define UTIL_CLOVER_FIELDHOLDER_HPP

#include "build.hpp"
#include "util/dyn_array.hpp"
#include "util/pack.hpp"
#include "util/string.hpp"
#include "game/worldentity_handle.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace clover {
namespace util {

// Keeps track of variables given to it
/// @todo Remove - this is from time before I knew templates
class FieldHolder {
public:

	class Field {
	public:

		enum Type {

#define TYPE(name_, type_) name_ ,

			#include "fh_types.def"
#undef TYPE

			LastType
		};

		// maxlen -1 == single variable
		Field(void *ptr, Type type, const util::Str8 name, int32 maxlen=-1);

		bool isArray() const {
			if (arrayLen > 0)return true;
			return false;
		}

		Type getType() const {
			return type;
		}

		uint32 getHash() const {
			return hash;
		}

		int32 getMaxCount() const {
			return arrayLen;
		}

		void *getPtr() const {
			return ptr;
		}

		template<typename T>
		void setChangeFunc(const boost::function<void (T val)> &t){
			switch (type){
#define TYPE(name_, type_)													\
					case name_: name_ ## ChangeFunc=						\
						*(boost::function<void (type_ val)>*)(&t); break;

				#include "fh_types.def"
#undef TYPE

				default: ensure(0);
			};
			changeFuncSet= true;
		}

		template<typename T>
		void setArrayChangeFunc(const boost::function<void (T val)> &t){
			switch (type){
#define TYPE(name_, type_)														\
					case name_: name_ ## ChangeFunc=							\
						*(boost::function<void (util::DynArray<type_>& val)>*)(&t);\
					break;
				#include "fh_types.def"
#undef TYPE


				default: ensure(0);
			};
			changeFuncSet= true;
		}


		bool hasChangeFunc() const {
			return changeFuncSet;
		}


#define TYPE(name_, type_)										\
			boost::function<void (type_ val)> name_ ## ChangeFunc;										\
			boost::function<void (util::DynArray<type_>& val)> name_ ## ArrayChangeFunc;
		#include "fh_types.def"
#undef TYPE
/*
		static constexpr uint8 subNone= 0;
		static constexpr uint8 subLooseHandle= 1;
		static constexpr uint8 subStrictHandle= 2;
*/
	protected:
		void *ptr;
		Type type;

		int32 arrayLen;
		
		// Hash of name
		uint32 hash;

		bool changeFuncSet;
	};

	FieldHolder();

#define TYPE(name_, type_) \
			virtual Field* add(type_ & ref, const util::Str8 &name/*, uint8 subtype=Field::subNone*/); \
			virtual Field* add(util::DynArray<type_> & ref, const util::Str8 &name, uint32 maxcount=1/*, uint8 subtype=Field::subNone*/);

	#include "fh_types.def"
#undef TYPE

	virtual Field* getFieldByName(const util::Str8& name);

	uint32 size(){
		return fields.size();
	}

	bool hasWEHandles(){
		return weHandleStored;
	}

protected:
	util::DynArray<Field> fields;
	bool weHandleStored;
};

} // util
} // clover

#endif // UTIL_CLOVER_FIELDHOLDER_HPP