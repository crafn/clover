#ifndef CLOVER_SCRIPT_OBJECTTYPE_HPP
#define CLOVER_SCRIPT_OBJECTTYPE_HPP

#include "build.hpp"
#include "exception.hpp"
#include "typestring.hpp"
#include "util/ensure.hpp"
#include "util/string.hpp"

#include <angelscript.h>

class asIObjectType;

namespace clover {
namespace script {

template <typename T>
class Function;

class ObjectType {
public:
	ObjectType(); // Invalid state
	ObjectType(asIObjectType& type_);
	ObjectType(const ObjectType& other);
	ObjectType(ObjectType&& other);
	virtual ~ObjectType();
	
	ObjectType& operator=(const ObjectType& other);
	ObjectType& operator=(ObjectType&& other);
	
	template <typename F>
	Function<F> getMethod(const util::Str8& name) const;
	
	asIObjectType& getAsType() const { return *type; }
	
	bool isGood() const { return type != nullptr; }
	
private:
	asIObjectType* type;
};

#include "objecttype.tpp"

} // script
} // clover

#endif // CLOVER_SCRIPT_OBJECTTYPE_HPP