#ifndef CLOVER_UTIL_DYN_ARRAY_HPP
#define CLOVER_UTIL_DYN_ARRAY_HPP

#include "build.hpp"
#include "util/arrayview.hpp"
#include "util/bool_wrap.hpp"
#include "util/containeralgorithms.hpp"
#include "util/ensure.hpp"
#include "util/hash.hpp"

#include <vector>
#include <algorithm>

namespace clover {
namespace util {

/// Substitute for std::vector
/// T2 should never be altered, it's a part of std::vector<bool> workaround
template <typename T, template <typename> class Ator= std::allocator, typename T2=T>
class DynArray {
public:
	using Value= T2;

	typedef Ator<T2> AtorT;
	typedef DynArray<T, Ator, T2> This;
	typedef typename std::vector<Value, AtorT>::iterator Iter;
	/// @todo Rename to match naming conventions
	typedef typename std::vector<Value, AtorT>::reverse_iterator rIter;
	typedef typename std::vector<Value, AtorT>::const_iterator cIter;
	typedef typename std::vector<Value, AtorT>::const_reverse_iterator crIter;

	DynArray()= default;

	DynArray(SizeType size)
			: c(size){
	}

	DynArray(std::initializer_list<T> l){
		c.insert(c.begin(), l.begin(), l.end());
	}

	DynArray(const AtorT& alloc)
			: c(alloc){
	}

	DynArray(const DynArray&)= default;
	DynArray(DynArray&&)= default;
	
	DynArray& operator=(const DynArray& t)= default;
	DynArray& operator=(DynArray&& t)= default;

	Value& operator[](SizeType i){ return c[i]; }
	const Value& operator[](SizeType i) const { return c[i]; }

	Value& at(SizeType i){ return c.at(i); }
	const Value& at(SizeType i) const { return c.at(i); }

	void pushBack(const T& t){ c.push_back(t); }
	
	void pushBack(T&& t){
		c.push_back(std::forward<T>(t));
	}

	void pushBack(const DynArray& v){
		for (auto& m : v)
			pushBack(m);
	}
	
	DynArray<T> pushBacked(const DynArray& v) const {
		DynArray<T> ret(*this);
		ret.pushBack(v);
		return (ret);
	}

	template<typename... Args>
	void emplaceBack(Args&&... args){
		c.emplace_back(std::forward<Args>(args)...);
	}
	
	SizeType size() const { return c.size(); }
	bool empty() const { return c.empty(); }
	void clear(){ c.clear(); }
	void resize(uint32 s){ c.resize(s); }
	void resize(uint32 s, const T& value){ c.resize(s, value); }
	void reserve(uint32 s){ c.reserve(s); }

	void sort(){ std::sort(c.begin(), c.end()); }
	
	Iter begin(){ return c.begin(); }
	Iter end(){ return c.end(); }
	cIter begin() const { return c.cbegin(); }
	cIter end() const { return c.cend(); }

	rIter rBegin(){ return c.rbegin(); }
	rIter rEnd(){ return c.rend(); }
	crIter rBegin() const { return c.crbegin(); }
	crIter rEnd() const { return c.crend(); }

	T* data(){ return c.data(); }
	const T* data() const { return c.data(); }

	T& front(){ return c.front(); }
	T& back(){ return c.back(); }
	const T& front() const { return c.front(); }
	const T& back() const {	return c.back(); }
	
	void insert(Iter it, const T& t){ c.insert(it, t); }
	void insert(Iter it, SizeType n, const T& t){ c.insert(it, n, t); }
	
	template <typename InputIter>
	void insert(Iter position, InputIter first, InputIter last){
		c.insert(position, first, last);
	}
	
	Iter erase(Iter it){ return c.erase(it); }
	Iter erase(Iter it, Iter it2){ return c.erase(it, it2); }
	
	void popBack(){ c.pop_back(); }
	void popFront(){ c.erase(begin()); }
	
	SizeType count(const T& t) const {
		uint32 a=0;
		for (uint32 i=0; i<c.size(); ++i)
			if(c[i] == t) ++a;
		return a;
	}
	
	cIter find(const T& t) const {
		for (cIter it= c.begin(); it != c.end(); ++it)
			if (*it == t) return it;
		return c.end();
	}
	
	Iter find(const T& t) {
		for (Iter it= c.begin(); it != c.end(); ++it)
			if (*it == t) return it;
		return c.end();
	}
	
	void remove(const T& t){
		auto it= find(t);
		debug_ensure(it != end());
		erase(it);
	}
	
	void append(const This& other){
		insert(end(), other.begin(), other.end());
	}

	template <typename Archive>
	void serialize(Archive& ar, const uint32 version){
		ar & c;
	}
	
	This duplicates(const This& other) const {
		return (util::duplicates(*this, other));
	}
	
	This removed(const This& other) const {
		return (util::removed(*this, other));
	}
	
	operator typename util::ArrayView<Value>(){
		return typename util::ArrayView<Value>(&*begin(), &*end());
	}

	operator typename util::ArrayView<const Value>() const {
		return util::ArrayView<const Value>(&*begin(), &*end());
	}

private:
	std::vector<Value, AtorT> c;
};

/// std::vector<bool> workaround
template <template <typename> class Ator>
class DynArray<bool, Ator,	bool> : public DynArray<bool, Ator, BoolWrap> {
};

template<typename T, template <typename> class Ator>
struct Hash32<DynArray<T, Ator>> {
	uint32 operator()(const DynArray<T, Ator>& arr) const {
		if (arr.empty()) return 0;
		return rawArrayHash(arr.data(), arr.size());
	}
};

template <typename T, template <typename> class Ator>
void fastInsert(DynArray<T, Ator>& container, T value){
	container.pushBack(std::move(value));
}

} // util
} // clover

#endif // CLOVER_UTIL_DYN_ARRAY_HPP
