#include "armatureattachment_def.hpp"
#include "global/exception.hpp"

namespace clover {
namespace util {

util::ObjectNode ObjectNodeTraits<visual::ArmatureAttachmentDef>::serialized(const Value& value){
	util::ObjectNode ob;
	ob["entity"].setValue(value.entityName);
	ob["joint"].setValue(value.jointName);
	ob["offset"].setValue(value.offset);
		
	return ob;
}

auto ObjectNodeTraits<visual::ArmatureAttachmentDef>::deserialized(const util::ObjectNode& ob) -> Value {
	if (!ob.isObject())
		throw global::Exception("Invalid ArmatureAttachmentDef");
	
	Value value;
	value.entityName= ob.get("entity").getValue<util::Str8>();
	value.jointName= ob.get("joint").getValue<util::Str8>();
	value.offset= ob.get("offset").getValue<visual::ArmatureAttachmentDef::Transform>();

	return value;
}

} // util
} // clover