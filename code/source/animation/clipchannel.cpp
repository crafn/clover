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
		case animation::ClipChannelType::rotation:
			ob["type"].
				setValue(util::Str8("rotation"));
		break;
		case animation::ClipChannelType::scale:
			ob["type"].
				setValue(util::Str8("scale"));
		break;
		default: fail("Unknown ClipChannelType");
	}
	ob["joint"].setValue(channel.joint);

	ObjectNode keys(ObjectNode::Value::Array);
	for (auto& m : channel.keys) {
		util::ObjectNode s;
		s["t"].setValue(m.time);
		util::ObjectNode values(ObjectNode::Value::Array);
		switch (channel.type) {
			case animation::ClipChannelType::translation:
				values.append(m.value[0]);
				values.append(m.value[1]);
				values.append(m.value[2]);
			break;
			case animation::ClipChannelType::rotation:
				values.append(m.value[0]);
				values.append(m.value[1]);
				values.append(m.value[2]);
				values.append(m.value[3]);
			break;
			case animation::ClipChannelType::scale:
				values.append(m.value[0]);
			break;
			default: fail("Unknown ClipChannelType");
		}
		s["v"]= values;
		keys.append(s);
	}
	ob["keys"]= keys;
	return ob;
}

auto ObjectNodeTraits<animation::ClipChannel>::
deserialized(const util::ObjectNode& ob) -> Value
{
	Value v;
	auto&& type_str= ob.get("type").getValue<util::Str8>();

	if (type_str == "translation")
		v.type= animation::ClipChannelType::translation;
	else if (type_str == "rotation")
		v.type= animation::ClipChannelType::rotation;
	else if (type_str == "scale")
		v.type= animation::ClipChannelType::scale;
	else
		fail("Unknown ClipChannelType");

	v.joint= ob.get("joint").getValue<util::Str8>();

	auto&& ob_keys= ob.get("keys");
	v.keys.resize(ob_keys.size());
	for (SizeType i= 0; i < ob_keys.size(); ++i) {
		v.keys[i].time= ob_keys.get(i).get("t").getValue<real32>();
		auto&& ob_v= ob_keys.get(i).get("v");
		switch (v.type) {
			case animation::ClipChannelType::translation: {
				v.keys[i].value[0]= ob_v.get(0).getValue<real32>();
				v.keys[i].value[1]= ob_v.get(1).getValue<real32>();
				v.keys[i].value[2]= ob_v.get(2).getValue<real32>();
			} break;
			case animation::ClipChannelType::rotation: {
				v.keys[i].value[0]= ob_v.get(0).getValue<real32>();
				v.keys[i].value[1]= ob_v.get(1).getValue<real32>();
				v.keys[i].value[2]= ob_v.get(2).getValue<real32>();
				v.keys[i].value[3]= ob_v.get(3).getValue<real32>();
			} break;
			case animation::ClipChannelType::scale: {
				v.keys[i].value[0]= ob_v.get(0).getValue<real32>();
			} break;
			default: fail("Unknown ClipChannelType");
		}
	}

	return v;
}

} // util
} // clover
