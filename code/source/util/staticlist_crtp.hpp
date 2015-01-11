#ifndef CLOVER_UTIL_STATICLIST_CRTP_HPP
#define CLOVER_UTIL_STATICLIST_CRTP_HPP

#include "build.hpp"
#include "util/linkedlist.hpp"

namespace clover {
namespace util {

/// CRTP for keeping track of instances of a single class with a static list
template <typename T>
class ENGINE_API StaticListCrtp {
public:
	using This= StaticListCrtp<T>;
	using List= util::LinkedList<T*>;

	StaticListCrtp(){ add(); }
	StaticListCrtp(const This&){ add(); }
	StaticListCrtp(This&& other){ other.remove(); add(); }
	~StaticListCrtp(){ remove(); }

	This& operator=(const This&)= default;
	This& operator=(This&& other){ other.remove(); return *this; }

	static const List& getInstances(){ return list; }

private:
	using Iter= typename util::LinkedList<T*>::Iter;

	void add(){
		iter= list.insert(list.end(), static_cast<T*>(this));
	}

	void remove(){
		if (iter != list.end()){
			list.erase(iter);
			iter= list.end();
		}
	}

	static List list;
	Iter iter;
};

template <typename T>
typename StaticListCrtp<T>::List StaticListCrtp<T>::list;

} // util
} // clover

#endif // CLOVER_UTIL_STATICLIST_CRTP_HPP
