#include "referencecountable.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace util {

ReferenceCountable::ReferenceCountable():
	refCount(0){
}

ReferenceCountable::~ReferenceCountable(){
	ensure(refCount == 0);
}
	
void ReferenceCountable::incrementReferenceCount() const {
	++refCount;
}

void ReferenceCountable::decrementReferenceCount() const {
	--refCount;
	ensure(refCount >= 0);
}

} // util
} // clover