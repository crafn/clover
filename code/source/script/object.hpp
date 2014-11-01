#ifndef CLOVER_SCRIPT_OBJECT_HPP
#define CLOVER_SCRIPT_OBJECT_HPP

#include "build.hpp"
#include "objecttype.hpp"

class asIScriptObject;

namespace clover {
namespace script {

/// Instantiated object in a script context
class Object {
public:
	enum RefType {
		Strong,
		Weak
	};

	Object(); // Invalid state
	Object(const ObjectType& type, asIScriptObject& as_obj, RefType ref_type= RefType::Strong);
	Object(const Object&)= delete; // Too lazy to implement
	Object(Object&&);
	virtual ~Object();
	
	Object& operator=(const Object&)= delete;
	Object& operator=(Object&&);
	
	const ObjectType& getType() const;
	asIScriptObject& getAsObject() const;
	
	bool isGood() const { return object != nullptr; }
	
private:
	void tryAddRef();
	void tryRelease();

	ObjectType type;
	asIScriptObject* object;
	RefType refType;
};

} // script
} // clover

#endif // CLOVER_SCRIPT_OBJECT_HPP