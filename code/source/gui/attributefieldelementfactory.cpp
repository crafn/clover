#include "attributefieldelementfactory.hpp"

#define ATTRIBUTE_FIELD_HEADERS
#include "resources/attributetypes.def"
#undef ATTRIBUTE_FIELD_HEADERS

namespace clover {
namespace gui {

BaseAttributeFieldElement* AttributeFieldElementFactory::create(const resources::AttributeDef& def){

	switch (def.getType().plainAttributeType){
		
		#define PLAIN_ATTRIBUTE(type) \
		case resources::PlainAttributeType::type: \
			if (def.getType().isArray){ \
				typedef resources::AttributeType<resources::PlainAttributeType::type, true> AttribType; \
				return new AttributeFieldElement<AttribType>(def); \
			} \
			else { \
				typedef resources::AttributeType<resources::PlainAttributeType::type, false> AttribType; \
				return new AttributeFieldElement<AttribType>(def); \
			}
		#include "resources/attributetypes.def"
		#undef PLAIN_ATTRIBUTE
		
		default: break;
	}
	
	throw global::Exception("Couldn't find specialization for %s", def.getType().getString().cStr());
}

} // gui
} // clover