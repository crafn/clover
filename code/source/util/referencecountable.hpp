#ifndef CLOVER_UTIL_REFERENCECOUNTABLE_HPP
#define CLOVER_UTIL_REFERENCECOUNTABLE_HPP

#include "build.hpp"

namespace clover {
namespace util {

template <typename T>
class CountedPointer;

/// Inherit from this if reference counting is needed for a type
/// An object must be then referenced by a Reference -class
class ReferenceCountable {
public:
	ReferenceCountable();
	virtual ~ReferenceCountable();

	int32 getReferenceCount() const { return refCount; }
	
private:
	template <typename T>
	friend  class CountedPointer;
	void incrementReferenceCount() const;
	void decrementReferenceCount() const;
	
	mutable int32 refCount;
};

} // util
} // clover

#endif // CLOVER_UTIL_REFERENCECOUNTABLE_HPP