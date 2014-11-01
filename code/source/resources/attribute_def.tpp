template <PlainAttributeType A>
auto AttributeDefImpl<AttributeType<A, true>>::deserialized(const AttributeParseInfo& a) -> Value {
	if (!a.object.isArray())
		throw ResourceException("Wrong json value type for attribute type Array");

	Value ret;
	for (SizeType i=0; i<a.object.size(); ++i){
		util::ObjectNode element= a.object.get(i);
		AttributeParseInfo single_info(element, a.resourceDirectory);
		ret.pushBack(ElementDefType::deserialized(single_info));
	}
	
	return ret;
}

template <PlainAttributeType A>
util::ObjectNode AttributeDefImpl<AttributeType<A, true>>::serialized(const Value& v){
	util::ObjectNode ret(util::ObjectNode::Value::Array);
	for (auto m : v){
		ret.append(ElementDefType::serialized(m));
	}
	return ret;
}