#ifndef CLOVER_UTIL_ANY_HPP
#define CLOVER_UTIL_ANY_HPP

#include "build.hpp"

#include <boost/any.hpp>

namespace clover {
namespace util {

using Any= boost::any;
using BadAnyCast= boost::bad_any_cast;

template <typename T>
T anyCast(Any& a){
	return boost::any_cast<T>(a);
}

template <typename T>
T anyCast(const Any& a){
	return boost::any_cast<T>(a);
}

template <typename T>
T* anyCast(Any* a){
	return boost::any_cast<T>(a);
}

template <typename T>
T* anyCast(const Any* a){
	return boost::any_cast<T>(a);
}

} // util
} // clover

#endif // CLOVER_UTIL_ANY_HPP
