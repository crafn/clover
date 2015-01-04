#ifndef CLOVER_UTIL_PTRTABLE_HPP
#define CLOVER_UTIL_PTRTABLE_HPP

#include "arrayview.hpp"
#include "build.hpp"
#include "global/exception.hpp"
#include "debug/debugprint.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace util {

/// Dynamic array containing pointers
template <typename T>
class PtrTable {
public:
	typedef typename util::DynArray<T*>::Iter Iter;
	typedef typename util::DynArray<T*>::cIter cIter;

	PtrTable(int32 size=1024){
		table.resize(size);
		for (int32 i=0; i<size; i++){
			table[i]= 0;
		}
	}

	void resize(int32 size){
		int32 oldsize= table.size();
		
		table.resize(size);

		for (int32 i=oldsize; i<size; i++)
			table[i]= 0;
	}

	int32 findFreeIndex(){
		for (uint32 i=0; i<table.size(); i++){
			if (table[i] == 0)return i;
		}

		print(debug::Ch::General, debug::Vb::Moderate,
			"PtrTable::getFreeIndex(): table is full (size: %lu)", (unsigned long)table.size());

		int32 s= table.size();
		s *= 2;
		if (s == 0)
			s=4;

		resize(s);
		return findFreeIndex();
	}

	T*& findFree(){
		for (int32 i=0; i<(int)table.size(); i++){
			if (table[i] == 0)return table[i];
		}

		print(debug::Ch::General, debug::Vb::Moderate, "PtrTable::getFreeIndex(): table is full (size: %i)", int(table.size()));

		int32 s= table.size();

		s *= 2;
		if (s == 0)s=4;

		resize(s);

		return findFree();
	}

	int32 findUsedIndex() const {
		for (SizeType i=0; i<table.size(); ++i){
			if (table[i]) return i;
		}
		return -1;
	}

	uint32 insert(T& t){
		uint32 i= findFreeIndex();
		table[i]= &t;
		return i;
	}

	void remove(uint32 index){
		table[index]= 0;
	}

	Iter begin() { return table.begin(); }
	Iter end() { return table.end(); }

	cIter begin() const { return table.begin(); }
	cIter end() const { return table.end(); }

	T* const * data() const { return table.data(); }
	T** data() { return table.data(); }
	
	uint32 size() const {
		return table.size();
	}

	T*& operator [](int index){
		return table[index];
	}

	T* operator [](int index) const {
		return table[index];
	}

	/// @return Count of non-null pointers
	uint32 getUsedCount() const {
		uint32 c= 0;
		for (auto m : table){
			if (m) ++c;
		}
		return c;
	}

private:
	util::DynArray<T*> table;
};

} // util
} // clover

#endif // CLOVER_UTIL_PTRTABLE_HPP
