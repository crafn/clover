#include "attribute_def.hpp"
#include "attributeparseinfo.hpp"
#include "collision/baseshape_circle.hpp"
#include "collision/baseshape_polygon.hpp"
#include "resources/exception.hpp"

#include <json/value.h>

namespace clover {
namespace resources {

// AttributeDefImpl

auto AttributeDefImpl<PathAttributeType>::deserialized(const AttributeParseInfo& a) -> Value {
	return Value(a.resourceDirectory, util::Str8(a.object.getValue<util::Str8>()));
		
	throw ResourceException("Wrong json value type for attribute-type Path");
}

util::ObjectNode AttributeDefImpl<PathAttributeType>::serialized(const Value& v){
	return (util::ObjectNode::create<util::Str8>(v.value()));
}


auto AttributeDefImpl<TriIndicesAttributeType>::deserialized(const AttributeParseInfo& a) -> Value {
	Value v;
	for (SizeType tri_i= 0; tri_i < a.object.size(); ++tri_i){
		if (!a.object.get(tri_i).isArray() || a.object.get(tri_i).size() != 3)
			throw ResourceException("Invalid index formatting");
			
		for (SizeType i= 0; i < 3; ++i){
			v.pushBack(a.object.get(tri_i).get(i).getValue<Value::Value>());
		}
	}
	
	return (v);
}

util::ObjectNode AttributeDefImpl<TriIndicesAttributeType>::serialized(const Value& v){
	util::ObjectNode ob(util::ObjectNode::Value::Array);
	ensure(v.size() % 3 == 0);
	
	SizeType tri_i= 0;
	for (SizeType i= 0; i+2 < v.size(); i += 3){
		util::ObjectNode inds;
		inds.append(v[i]);
		inds.append(v[i+1]);
		inds.append(v[i+2]);
		ob.append(inds); // Array of arrays
	}
	
	return (ob);
}

// AttributeDef

AttributeDef::AttributeDef(const BaseAttributeDefImpl& impl)
	: AttributeDef(impl.key, impl.clone()){
	
}

AttributeDef AttributeDef::String(const util::Str8& key, const util::DynArray<util::Str8>& autofill, uint32 maxlen){
	AttributeDef def(key, new AttributeDefImpl<StringAttributeType>(autofill, maxlen));
	return (def);
}

AttributeDef AttributeDef::Resource(const util::Str8& key, const util::Str8& autosearch_tags){
	AttributeDef def(key, new AttributeDefImpl<ResourceAttributeType>(autosearch_tags));
	return (def);
}

AttributeDef AttributeDef::ResourcePair(const util::Str8& key, const util::Str8& autosearch_tags){
	AttributeDef def(key, new AttributeDefImpl<ResourcePairAttributeType>(autosearch_tags));
	return (def);
}

AttributeDef AttributeDef::Path(const util::Str8& key){
	AttributeDef def(key, new AttributeDefImpl<PathAttributeType>());
	return (def);
}

AttributeDef AttributeDef::Real(const util::Str8& key, bool integer, real64 minvalue, real64 maxvalue){
	AttributeDef def(key, new AttributeDefImpl<RealAttributeType>(integer, minvalue, maxvalue));
	return (def);
}

AttributeDef AttributeDef::Vector2(const util::Str8& key){
	AttributeDef def(key, new AttributeDefImpl<Vector2AttributeType>());
	return (def);
}

AttributeDef AttributeDef::Vector4(const util::Str8& key){
	AttributeDef def(key, new AttributeDefImpl<Vector4AttributeType>());
	return (def);
}

AttributeDef AttributeDef::Boolean(const util::Str8& key){
	AttributeDef def(key, new AttributeDefImpl<BooleanAttributeType>());
	return (def);
}

AttributeDef AttributeDef::Color(const util::Str8& key, bool has_alpha){
	AttributeDef def(key, new AttributeDefImpl<ColorAttributeType>(has_alpha));
	return (def);
}

AttributeDef AttributeDef::SignalArgument(const util::Str8& key){
	AttributeDef def(key, new AttributeDefImpl<SignalArgumentAttributeType>());
	return (def);
}

AttributeDef AttributeDef::SignalArgumentArray(const util::Str8& key){
	return AttributeDef(key, new AttributeDefImpl<SignalArgumentArrayAttributeType>(AttributeDefImpl<SignalArgumentAttributeType>()));
}

AttributeDef::AttributeDef(const util::Str8& key_, BaseAttributeDefImpl* impl_):
	type(impl_->getAttributeType()),
	impl(std::shared_ptr<BaseAttributeDefImpl>(impl_)){
	
	impl->key= key_;
}

} // resources
} // clover