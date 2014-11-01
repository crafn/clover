#ifndef CLOVER_SCRIPT_REFERENCE_HPP
#define CLOVER_SCRIPT_REFERENCE_HPP

#include "build.hpp"

namespace clover {
namespace script {

/// Manual reference counting required by angelscript for handle objects
/// Deletes itsef when count reaches zero
/// Usage: 	class A : public script::Reference {};
///			scriptMgr.registerObjectType<A>();
/// @todo Must rethink. Don't use! Had some mysterious crashes.
class Reference {
public:
	
	Reference();
	Reference(const Reference& other);
	Reference(Reference&& other);
	virtual ~Reference();
	
	Reference& operator=(const Reference& other);
	Reference& operator=(Reference&& other);
	
	void addRef();
	void release(); // Deletes this if refCount reaches zero
	
private:
	int32 refCount;
};

/// Constructing and destructing controlled by native code
/// @todo Remove. Use registerObjectType<Type*>() instead
class NoCountReference {
public:
	virtual ~NoCountReference(){}
};

} // script
} // clover

#endif // CLOVER_SCRIPT_REFERENCE_HPP