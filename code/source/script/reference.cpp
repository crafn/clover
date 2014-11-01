#include "reference.hpp"
#include "debug/debugprint.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace script {

Reference::Reference()
	: refCount(1){
	ensure(0);
}

Reference::Reference(const Reference& other)
	: refCount(1){
	ensure(0);
}

Reference::Reference(Reference&& other)
	: refCount(1){
	/// @todo Rethink
	ensure(0);
}

Reference::~Reference(){
}

Reference& Reference::operator=(const Reference& other){
	// Do nothing
	return *this;
}

Reference& Reference::operator=(Reference&& other){
	refCount= other.refCount;
	other.refCount= 0;
	return *this;
}

void Reference::addRef(){
	++refCount;
}

void Reference::release(){
	--refCount;
	print(debug::Ch::General, debug::Vb::Trivial, "release %p, refcount: %i", this, refCount);
	if (refCount == 0)
		delete this;
}

} // script
} // clover