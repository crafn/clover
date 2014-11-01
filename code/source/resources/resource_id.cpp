#include "resource_id.hpp"
#include "resource.hpp"

namespace clover {
namespace resources {

ResourceId::ResourceId(const Resource& res)
		: typeName(res.getResourceTypeName())
		, identifier(res.getIdentifierAttribute().serialized()){
}

ResourceId::ResourceId(util::Str8 type_name, const util::ObjectNode& id)
		: typeName(type_name)
		, identifier(id){

}

} // resources
} // clover