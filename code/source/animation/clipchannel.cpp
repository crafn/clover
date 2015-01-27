#include "animation/clipchannel.hpp"
#include "util/objectnode.hpp"

namespace clover {
namespace animation {


} // animation
namespace util {

util::ObjectNode ObjectNodeTraits<animation::ClipChannel>::
serialized(const Value& channel)
{
	util::ObjectNode ob;
	switch (channel.type) {
		case animation::ClipChannelType::translation:
			ob["type"].
				setValue(util::Str8("translation"));
		break;
		default: fail("Unknown ClipChannelType");
	}
	ob["joint"].setValue(channel.joint);

	auto&& keys= ob["keys"]= ObjectNode(ObjectNode::Value::Array);
	for (auto& m : channel.keys) {
		util::ObjectNode s;
		s["t"].setValue(m.time);
		auto&& values= s["v"]= ObjectNode(ObjectNode::Value::Array);
		switch (channel.type) {
			case animation::ClipChannelType::translation:
				values.append(m.value[0]);
				values.append(m.value[1]);
				values.append(m.value[2]);
			break;
			default: fail("Unknown ClipChannelType");
		}
		keys.append(s);
	}

	return ob;
}

auto ObjectNodeTraits<animation::ClipChannel>::
deserialized(const util::ObjectNode& ob) -> Value
{
	Value v;
	auto&& type_str= ob.get("type").getValue<util::Str8>();

	if (type_str == "translation")
		v.type= animation::ClipChannelType::translation;
	else
		fail("Unknown ClipChannelType");

	v.joint= ob.get("joint").getValue<util::Str8>();

	auto&& ob_keys= ob.get("keys");
	v.keys.resize(ob_keys.size());
	for (SizeType i= 0; i < ob_keys.size(); ++i) {
		switch (v.type) {
			case animation::ClipChannelType::translation: {
				v.keys[i].time= ob_keys.get(i).get("t").getValue<real32>();
				auto&& ob_v= ob_keys.get(i).get("v");
				v.keys[i].value[0]= ob_v.get(0).getValue<real32>();
				v.keys[i].value[1]= ob_v.get(1).getValue<real32>();
				v.keys[i].value[2]= ob_v.get(2).getValue<real32>();
			} break;
			default: fail("Unknown ClipChannelType");
		}
	}

	return v;
}

} // util
} // clover
