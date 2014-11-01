#include "msg.hpp"
#include "util/pack.hpp"

namespace clover {
namespace net { namespace msg {

Msg::Name Msg::literalToName(const char* s){
	ensure_msg(strlen(s) == Msg::nameSize, "strlen(\"%s\") != %i", s, Msg::nameSize); // Couldn't figure out compile time check
	
	Msg::Name ret;
	for (SizeType i= 0; i < ret.size(); ++i)
		ret[i]= static_cast<uint8>(s[i]);
	return ret;
}

Msg::RawValueSize Msg::rawValueSize(const RawRawValueSize& raw_size){
	util::RawPack size_pack;
	for (const auto& m : raw_size)
		size_pack << m;

	RawValueSize size;
	size_pack >> size;
	return size;
}

Msg::Msg(const Name& name_)
	: name(name_){}

Msg::RawMsg Msg::getRawMsg() const {
	SizeType raw_value_size= rawValue.size();
	ensure(raw_value_size < std::numeric_limits<RawValueSize>::max());
	
	util::RawPack size_pack;
	size_pack << (RawValueSize)raw_value_size;
	
	RawMsg ret;
	for (auto& m : name)
		ret.pushBack(m);
	ret.pushBack(size_pack.getData());
	ret.pushBack(rawValue);
	return (ret);
}

}} // net::msg
} // clover