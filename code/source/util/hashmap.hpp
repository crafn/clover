#ifndef CLOVER_UTIL_HASHMAP_HPP
#define CLOVER_UTIL_HASHMAP_HPP

#include "build.hpp"
#include "containeralgorithms.hpp"
#include "hash.hpp"

#include <unordered_map>

namespace clover {
namespace util {

template <typename K, typename V>
class HashMap {
public:
	typedef HashMap<K, V> This;

	typedef typename std::unordered_map<K, V, Hash32<K>>::iterator Iter;
	typedef typename std::unordered_map<K, V, Hash32<K>>::const_iterator cIter;


	V& operator[](const K& k){
		return c[k];
	}

	V& at(const K& k){
		return c.at(k);
	}

	Iter find(const K& k) {
		return c.find(k);
	}

	cIter find(const K& k) const {
		return c.find(k);
	}
	
	V get(const K& k, const V& error_v) const {
		auto it= find(k);
		if (it == c.end())
			return error_v;
		return it->second;
	}
	
	uint32 count(const K& k) const {
		return c.count(k);
	}
	
	uint32 size() const {
		return c.size();
	}
	
	bool empty() const {
		return c.empty();
	}



	std::pair<Iter, bool> insert(const std::pair<const K, V>& p ){
		return c.insert(p);
	}

	std::pair<Iter, bool> insert(std::pair<K, V>&& p){
		return c.insert(std::move(p));
	}
	
	template <typename It>
	void insert(It first, It last){
		c.insert(first, last);
	}

	void erase(Iter it){
		c.erase(it);
	}

	Iter end(){ return c.end(); }
	cIter end() const { return c.end(); }
	
	Iter begin(){ return c.begin(); }
	cIter begin() const { return c.begin(); }

	This duplicates(const This& other) const {
		return (util::duplicates(*this, other));
	}
	
	This removed(const This& other) const {
		return (util::removed(*this, other));
	}

private:
	std::unordered_map<K, V, Hash32<K>> c;
};

} // util
} // clover

#endif // CLOVER_UTIL_HASHMAP_HPP