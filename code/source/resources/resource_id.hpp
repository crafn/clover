#ifndef CLOVER_RESOURCES_RESOURCE_ID_HPP
#define CLOVER_RESOURCES_RESOURCE_ID_HPP

#include "build.hpp"
#include "util/objectnode.hpp"

namespace clover {
namespace resources {

class Resource;

/// Differs from ResourceTraits<T>::Identifier by including resource type as runtime data
class ResourceId {
public:

	ResourceId(const Resource& res);
	ResourceId(util::Str8 type_name, const util::ObjectNode& id);

	const util::Str8& getTypeName() const { return typeName; }
	const util::ObjectNode& getIdentifier() const { return identifier; }

private:
	util::Str8 typeName;
	util::ObjectNode identifier;
};

} // resources
namespace util {

template <>
struct ObjectNodeTraits<resources::ResourceId> {
	using Value= resources::ResourceId;
	static util::ObjectNode serialized(const Value& value){
		util::ObjectNode ob;
		ob["typeName"].setValue(value.getTypeName());
		ob["identifier"].setValue(value.getIdentifier());
		return (ob);
	}
	
	static Value deserialized(const util::ObjectNode& ob){
		Value v(ob.get("typeName").getValue<util::Str8>(), ob.get("identifier"));
		return (v);
	}
};

} // util
} // clover

#endif // CLOVER_RESOURCES_RESOURCE_ID_HPP