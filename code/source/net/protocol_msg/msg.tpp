template <typename Value>
void Msg::setValue(const Value& value){
	try {
		util::ObjectNode ob= util::ObjectNode::create(value);
		util::Str8 str= std::move(ob.generateText());
		
		rawValue.clear();
		for (SizeType i= 0; i < str.sizeBytes(); ++i)
			rawValue.pushBack(str[i]);
	}
	catch(const std::exception& e){
		throw NetException("Error serializing a value for Msg %s: %s", getName().data(), e.what());
	}
}

template <typename Value>
Value Msg::getValue() const {
	try {
		util::Str8 str;
		for (const auto& m : rawValue)
			str += m;
	
		util::ObjectNode ob;
		ob.parseText(str);
		return (ob.getValue<Value>());
	}
	catch(const std::exception& e){
		throw NetException("Error deserializing a value for Msg %s: %s", getName().data(), e.what());
	}
}