#ifndef CLOVER_UTIL_COUNTEDPOINTER_HPP
#define CLOVER_UTIL_COUNTEDPOINTER_HPP

#include "build.hpp"
#include "referencecountable.hpp"

namespace clover {
namespace util {

/// Manages ReferenceCountable's increment and decrement
/// Doesn't delete or allocate anything. Use smart pointers for that kind of stuff
template <typename T> // T implements ReferenceCountable
class CountedPointer {
public:
	CountedPointer(T* v= nullptr);
	CountedPointer(const CountedPointer&);
	CountedPointer(CountedPointer&&);
	
	virtual ~CountedPointer();
	
	CountedPointer& operator=(T*);
	CountedPointer& operator=(const CountedPointer&);
	CountedPointer& operator=(CountedPointer&&);
	
	bool operator==(const CountedPointer&) const;
	bool operator==(T*) const;
	
	/// True if not a nullptr
	explicit operator bool() const;
	
	T* operator->();
	const T* operator->() const;

	T& operator*();
	const T& operator*() const;

	T* get();
	const T* get() const;
	
private:
	T* ptr;
	
	void incr();
	void decr();
};

#include "countedpointer.tpp"

} // util
} // clover

#endif // CLOVER_UTIL_COUNTEDPOINTER_HPP