#ifndef CLOVER_UTIL_CONTAINERALGORITHMS_HPP
#define CLOVER_UTIL_CONTAINERALGORITHMS_HPP

#include "build.hpp"
#include "util/traits.hpp"

#include <algorithm>
#include <utility>

namespace clover {
namespace util {

template <typename C, typename T>
bool contains(const C& container, const T& contained){
	return container.find(contained) != container.end();
}

/// @return true if elements of 'contained' are contained in 'container'
template <typename T>
bool fullyContains(const T& container, const T& contained){
	for (const auto& to_find : contained){
		if (container.find(to_find) == container.end())
			return false;
	}
	return true;
}

template <typename T>
bool overlaps(const T& c1, const T& c2){
	for (auto& e1 : c1){
		for (auto& e2 : c2){
			if (e1 == e2)
				return true;
		}
	}
	return false;
}

template <typename T>
T duplicates(const T& t1, const T& t2){
	T ret;

	for (const auto& m : t1){
		for (const auto& m2 : t2){
			if (m == m2){
				ret.pushBack(m);
			}
		}
	}
	
	return ret;
}

template <typename T>
T removed(const T& from_here, const T& by_these){
	T ret;
	for (const auto& m : from_here){
		if (!contains(by_these, m))
			ret.insert(ret.end(), m);
	}
	return ret;
}

template <typename T, typename F>
T removedIf(const T& from_here, F condition){
	T ret;
	for (const auto& m : from_here){
		if (!condition(m))
			ret.insert(ret.end(), m);
	}
	return ret;
}

template <typename T>
T duplicatesRemoved(const T& container){
	T ret;
	for (auto it= container.begin(); it != container.end(); ++it){
		bool found= false;
		for (auto it2= std::next(it); it2 != container.end(); ++it2){
			if (*it == *it2){
				found= true;
				break;
			}
		}
		if (!found)
			ret.pushBack(*it);
	}
	return (ret);
}

/// Extracts elements from container for which condition is true
template <typename T, typename F>
T extractIf(T& container, F condition){
	T ret;
	for (auto it= container.begin(); it != container.end();){
		if (condition(*it)){
			ret.insert(ret.end(), std::move(*it));
			it= container.erase(it);
		}
		else {
			++it;
		}
	}
	return ret;
}

/// @example convertContained(util::Set{0,1,2}, [] (int n){ return n > 0; }) == util::Set{false, true}
template <typename C1, typename F>
typename SetParamPack<typename std::remove_const<C1>::type, typename util::FunctionTraits<F>::Return>::Type
convertContained(C1& c1, F&& conversion){
	using E2= typename util::FunctionTraits<F>::Return;
	typename SetParamPack<typename std::remove_const<C1>::type, E2>::Type c2;
	for (auto& e1 : c1)
		c2.insert(c2.end(), conversion(e1));
	return c2;
}

/// Containers not supporting insert(v) should have their own versions
template <typename C, typename T>
void fastInsert(C& container, T value){
	container.insert(std::move(value));
}

} // util
} // clover

#endif // CLOVER_UTIL_CONTAINERALGORITHMS_HPP
