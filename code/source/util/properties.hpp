#ifndef CLOVER_UTIL_PROPERTIES_HPP
#define CLOVER_UTIL_PROPERTIES_HPP

#include "any.hpp"
#include "build.hpp"
#include "hashmap.hpp"
#include "string.hpp"

namespace clover {
namespace util {

/// Contains a type-safe set of util::Any key-value pairs
class Properties {
public:

	template <typename T>
	void set(const Str8& key, T&& t)
	{
		map[key]= t;
	}

	template <typename T>
	const T* find(const Str8& key) const 
	{
		auto it= map.find(key);
		if (it == map.end())
			return nullptr;
		return anyCast<const T>(&it->second);
	}

	template <typename T>
	T* find(const Str8& key)
	{ return const_cast<T*>(static_cast<const Properties&>(*this).find<T>(key)); }

	template <typename T>
	const T& get(const Str8& key) const
	{ return *NONULL(find<T>(key)); }

	template <typename T>
	T& get(const Str8& key)
	{ return *NONULL(find<T>(key)); }

private:
	HashMap<Str8, Any> map;
};

template <typename T>
T tryGet(const Properties& props, const Str8& key, const T& not_found){
	auto found= props.find<T>(key);
	return found ? *found : not_found;
}

} // util
} // clover

#endif // CLOVER_UTIL_PROPERTIES_HPP
