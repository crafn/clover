#ifndef CLOVER_NET_PROTOCOL_MSG_MSG_HPP
#define CLOVER_NET_PROTOCOL_MSG_MSG_HPP

#include "../exception.hpp"
#include "../net.hpp"
#include "build.hpp"
#include "util/dyn_array.hpp"
#include "util/objectnode.hpp"

namespace clover {
namespace net { namespace msg {

/// Message structure in msg protocol:
/// - first 4 bytes == msg name
/// - following 8 bytes == uint32 valueSize
/// - following valueSize bytes == value (in util::ObjectNode -format)
class Msg {
public:
	static constexpr SizeType nameSize= 4; // 4 character long name
	using RawValueSize= uint32; // Allows 4 gigabyte large values :p
	using Name= std::array<uint8, Msg::nameSize>;
	using RawRawValueSize= std::array<uint8, sizeof(RawValueSize)>;
	using RawValue= util::DynArray<uint8>;
	using RawMsg= util::DynArray<uint8>;

	static Name literalToName(const char* s);
	static RawValueSize rawValueSize(const RawRawValueSize& raw_size);

	Msg(const Name& name_);

	const Name& getName() const { return name; }

	template <typename Value>
	void setValue(const Value& value);

	template <typename Value>
	Value getValue() const;

	void setRawValue(const RawValue& raw_value){ rawValue= raw_value; }
	RawValue getRawValue() const { return rawValue; }

	/// @return Whole msg in sendable form
	RawMsg getRawMsg() const;

	template <typename Archive>
	void serialize(Archive& ar, const uint32 ver){
		ar & getRawMsg();
	}

private:
	Name name;
	RawValue rawValue;
};

#define NET_MSG_TRAITS_NAME(x) \
	static net::msg::Msg::Name name(){ return net::msg::Msg::literalToName(x); }
#define NET_MSG_TRAITS_VALUE(x) \
	using Value= x;

struct PingMsgTraits {
	NET_MSG_TRAITS_NAME("ping")
	NET_MSG_TRAITS_VALUE(void)
};

struct PongMsgTraits {
	NET_MSG_TRAITS_NAME("pong")
	NET_MSG_TRAITS_VALUE(void)
};

#include "msg.tpp"

}} // net::msg
} // clover

#endif // CLOVER_NET_PROTOCOL_MSG_MSG_HPP