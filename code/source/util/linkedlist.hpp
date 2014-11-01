#ifndef CLOVER_UTIL_LIST_HPP
#define CLOVER_UTIL_LIST_HPP

#include "build.hpp"

#include <list>

namespace clover {
namespace util {

template <typename T, template <typename> class Ator= std::allocator>
class LinkedList {
public:
	using Value= T;
	
	typedef Ator<T> AtorT;
	typedef typename std::list<T>::iterator Iter;
	typedef typename std::list<T>::reverse_iterator rIter;
	typedef typename std::list<T>::const_iterator cIter;
	typedef typename std::list<T>::const_reverse_iterator crIter;

	LinkedList()= default;
	LinkedList(const LinkedList&)= default;

	LinkedList(LinkedList&&)= default;

	LinkedList& operator=(LinkedList&&)= default;
	LinkedList& operator=(const LinkedList&)= default;
	
	void pushBack(const T& t){
		c.push_back(t);
	}
	
	// Move
	void pushBack(T&& t){
		c.push_back(std::move(t));
	}
	
	void pushFront(const T& t){
		c.push_front(t);
	}
	
	// Move
	void pushFront(T&& t){
		c.push_front(std::move(t));
	}
	
	template<typename... Args>
	void emplaceBack(Args&&... args){
		c.emplace_back(std::forward<Args>(args)...);
	}

	uint32 size() const {
		return c.size();
	}
	
	bool empty() const {
		return c.empty();
	}
	
	void clear(){
		c.clear();
	}
	
	void sort(){
		c.sort();
	}
	
	Iter begin(){
		return c.begin();
	}
	Iter end(){
		return c.end();
	}
	
	rIter rBegin(){
		return c.rbegin();
	}
	rIter rEnd(){
		return c.rend();
	}

	cIter begin() const{
		return c.begin();
	}
	cIter end() const{
		return c.end();
	}
	
	crIter rBegin() const{
		return c.rbegin();
	}
	crIter rEnd() const{
		return c.rend();
	}
	
	T& front(){
		return c.front();
	}
	
	T& back(){
		return c.back();
	}
	
	const T& front() const {
		return c.front();
	}
	
	const T& back() const {
		return c.back();
	}
	
	void popFront(){
		c.pop_front();
	}
	
	void popBack(){
		c.pop_back();
	}

	Iter insert(cIter it, const T& t){ return c.insert(it, t); }
	Iter insert(cIter it, T&& t){ return c.insert(it, std::move(t)); }	
	Iter insert(Iter it, const T& t){ return c.insert(it, t); }
	Iter insert(Iter it, T&& t){ return c.insert(it, std::move(t)); }	

	template <class InputIterator>
    void insert ( Iter position, InputIterator first, InputIterator last ){
		c.insert(position, first, last);
	}
	
	Iter erase(const Iter& it){
		return c.erase(it);
	}
	
	
	Iter erase(const cIter& it){
		return c.erase(it);
	}
	
	void remove(const T& value){
		c.remove(value);
	}
	
	cIter find(const T& value) const {
		for (cIter it= c.begin(); it != c.end(); ++it)
			if (*it == value) return it;
		return c.end();
	}
	
	Iter find(const T& value){
		for (Iter it= c.begin(); it != c.end(); ++it)
			if (*it == value) return it;
		return c.end();
	}
	
private:
	std::list<T, AtorT> c;
};

} // util
} // clover

#endif // CLOVER_UTIL_LIST_HPP