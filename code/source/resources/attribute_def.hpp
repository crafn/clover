#ifndef CLOVER_RESOURCES_ATTRIBUTE_DEF_HPP
#define CLOVER_RESOURCES_ATTRIBUTE_DEF_HPP

#include "animation/joint_def.hpp"
#include "animation/clipchannel.hpp"
#include "attributeparseinfo.hpp"
#include "attributetype.hpp"
#include "build.hpp"
#include "collision/baseshape.hpp"
#include "exception.hpp"
#include "nodes/signalargument.hpp"
#include "pathattributevalue.hpp"
#include "physics/entityobject_def.hpp"
#include "physics/entityjoint_def.hpp"
#include "resourcepair.hpp"
#include "util/dyn_array.hpp"
#include "util/ensure.hpp"
#include "util/objectnode.hpp"
#include "util/math.hpp"
#include "util/string.hpp"
#include "visual/vertex.hpp"
#include "visual/armatureattachment_def.hpp"

#include <memory>

namespace clover {
namespace resources {

struct AttributeParseInfo;

template <typename T>
struct AttributeDefImpl; //{
//		typedef int64 Value;
//		static Value extractValue(const AttributeParseInfo&);
//		static Json::Value asJsonValue(const Value&);
// };

/// Contains information for attribute usage and restrictions in editor
/// @todo Rename *Impl to something more describing
/// @todo No, remove this overly complicated construction
struct BaseAttributeDefImpl {
	virtual ~BaseAttributeDefImpl(){}

	virtual RuntimeAttributeType getAttributeType() const = 0;

	virtual BaseAttributeDefImpl* clone() const = 0;

	util::Str8 key;
};


template <typename T>
struct AttributeDefImplWrap : public BaseAttributeDefImpl {
	// Provide type-information
	typedef T AttributeType;
	virtual RuntimeAttributeType getAttributeType() const { return T::runtimeType(); }

};

#define DECLARE_ATTRIBUTE_DEF_IMPL(x) \
	AttributeDefImpl(const AttributeDefImpl& other)= default; \
	virtual AttributeDefImpl* clone() const { return new AttributeDefImpl(*this); }

#define GENERIC_ATTRIBUTE_DEF_IMPL(AttribType, ValueType) \
	template<> \
	struct AttributeDefImpl<AttribType> : public AttributeDefImplWrap<AttribType> { \
		DECLARE_ATTRIBUTE_DEF_IMPL(AttribType) \
		using Value= ValueType; \
		static Value deserialized(const AttributeParseInfo& a){ return a.object.getValue<Value>(); } \
		static util::ObjectNode serialized(const Value& v){ return util::ObjectNode::create<Value>(v); } \
		static Value initValue(){ return Value(); } \
	};

template <>
struct AttributeDefImpl<StringAttributeType> : public AttributeDefImplWrap<StringAttributeType> {
	DECLARE_ATTRIBUTE_DEF_IMPL(StringAttributeType)

	AttributeDefImpl(const util::DynArray<util::Str8>& autofill= {}, uint32 maxlen= 1024):
		autoFill(autofill),
		maxLength(maxlen){}
	virtual ~AttributeDefImpl(){}

	typedef util::Str8 Value;
	static Value deserialized(const AttributeParseInfo& a){ return a.object.getValue<Value>(); }
	static util::ObjectNode serialized(const Value& v){ return (util::ObjectNode::create<Value>(v)); }

	static Value initValue(){ return Value(""); }

	util::DynArray<util::Str8> autoFill;
	uint32 maxLength;
};

template <>
struct AttributeDefImpl<ResourceAttributeType> : public AttributeDefImplWrap<ResourceAttributeType> {
	DECLARE_ATTRIBUTE_DEF_IMPL(ResourceAttributeType)

	AttributeDefImpl(const util::Str8& search_tags= ""):
		searchTags(search_tags){}
	virtual ~AttributeDefImpl(){}

	// If an arbitary identifier-type is needed,
	// implement something like a "class ResourceIdentity", which holds util::Any, and use it as Value
	typedef util::Str8 Value;
	static Value deserialized(const AttributeParseInfo& a){ return a.object.getValue<Value>(); }
	static util::ObjectNode serialized(const Value& v){ return (util::ObjectNode::create<Value>(v)); }

	static Value initValue(){ return Value(""); }

	// Tags of resource types which are meant for this attribute
	/// @todo Change to something which doesn't break if the tag is changed for some resource-type
	util::Str8 searchTags;
};


template <>
struct AttributeDefImpl<ResourcePairAttributeType> : public AttributeDefImplWrap<ResourcePairAttributeType> {
	DECLARE_ATTRIBUTE_DEF_IMPL(ResourcePairAttributeType)

	AttributeDefImpl(const util::Str8& search_tags= ""):
		searchTags(search_tags){}
	virtual ~AttributeDefImpl(){}

	typedef StrResourcePair Value;
	static Value deserialized(const AttributeParseInfo& a){ return a.object.getValue<Value>(); }
	static util::ObjectNode serialized(const Value& v){ return (util::ObjectNode::create<Value>(v)); }

	static Value initValue(){ return Value("", ""); }

	util::Str8 searchTags;
};

template <>
struct AttributeDefImpl<PathAttributeType> : public AttributeDefImplWrap<PathAttributeType> {
	DECLARE_ATTRIBUTE_DEF_IMPL(PathAttributeType)

	AttributeDefImpl(){}
	virtual ~AttributeDefImpl(){}

	typedef PathAttributeValue Value;
	static Value deserialized(const AttributeParseInfo&);
	static util::ObjectNode serialized(const Value&);

	static Value initValue(){ return Value(""); }
};

template<>
struct AttributeDefImpl<RealAttributeType> : public AttributeDefImplWrap<RealAttributeType> {
	DECLARE_ATTRIBUTE_DEF_IMPL(RealAttributeType)

	AttributeDefImpl(bool integer= false, real64 minvalue= -1000000, real64 maxvalue= 1000000):
	minValue(minvalue),
	maxValue(maxvalue){}

	virtual ~AttributeDefImpl(){}

	typedef real64 Value;
	static Value deserialized(const AttributeParseInfo& a){ return a.object.getValue<Value>(); }
	static util::ObjectNode serialized(const Value& v){ return util::ObjectNode::create<Value>(v); }

	static Value initValue(){ return 0.0; }

	Value minValue, maxValue;
};

template<>
struct AttributeDefImpl<IntegerAttributeType> : public AttributeDefImplWrap<IntegerAttributeType> {
	DECLARE_ATTRIBUTE_DEF_IMPL(RealAttributeType)

	AttributeDefImpl(int64 minvalue= -1000000, int64 maxvalue= 1000000)
		: minValue(minvalue)
		, maxValue(maxvalue){}

	virtual ~AttributeDefImpl(){}

	typedef int64 Value;
	static Value deserialized(const AttributeParseInfo& a){ return a.object.getValue<Value>(); }
	static util::ObjectNode serialized(const Value& v){ return util::ObjectNode::create<Value>(v); }

	static Value initValue(){ return 0; }

	Value minValue, maxValue;
};

template<>
struct AttributeDefImpl<Vector2AttributeType> : public AttributeDefImplWrap<Vector2AttributeType> {
	DECLARE_ATTRIBUTE_DEF_IMPL(Vector2AttributeType)

	AttributeDefImpl(){}
	virtual ~AttributeDefImpl(){}

	typedef util::Vec2d Value;
	static Value deserialized(const AttributeParseInfo& a){ return a.object.getValue<Value>(); }
	static util::ObjectNode serialized(const Value& v){ return util::ObjectNode::create<Value>(v); }

	static Value initValue(){ return Value(0.0); }
};

template<>
struct AttributeDefImpl<Vector4AttributeType> : public AttributeDefImplWrap<Vector4AttributeType> {
	DECLARE_ATTRIBUTE_DEF_IMPL(Vector4AttributeType)

	AttributeDefImpl(){}
	virtual ~AttributeDefImpl(){}

	typedef util::Vec4d Value;
	static Value deserialized(const AttributeParseInfo& a){ return a.object.getValue<Value>(); }
	static util::ObjectNode serialized(const Value& v){ return util::ObjectNode::create<Value>(v); }

	static Value initValue(){ return Value(0.0); }
};

template<>
struct AttributeDefImpl<BooleanAttributeType> : public AttributeDefImplWrap<BooleanAttributeType> {
	DECLARE_ATTRIBUTE_DEF_IMPL(BooleanAttributeType)

	AttributeDefImpl(){}
	virtual ~AttributeDefImpl(){}

	typedef bool Value;
	static Value deserialized(const AttributeParseInfo& a){ return a.object.getValue<Value>(); }
	static util::ObjectNode serialized(const Value& v){ return util::ObjectNode::create<Value>(v); }

	static Value initValue(){ return false; }
};

template<>
struct AttributeDefImpl<ColorAttributeType> : public AttributeDefImplWrap<ColorAttributeType> {
	DECLARE_ATTRIBUTE_DEF_IMPL(ColorAttributeType)

	AttributeDefImpl(bool alpha= true):
		hasAlpha(alpha){}

	virtual ~AttributeDefImpl(){}

	typedef util::Color Value;
	static Value deserialized(const AttributeParseInfo& a){ return a.object.getValue<Value>(); }
	static util::ObjectNode serialized(const Value& v){ return util::ObjectNode::create<Value>(v); }

	static Value initValue(){ return Value(); }

	bool hasAlpha;
};

template<>
struct AttributeDefImpl<BaseShapeAttributeType> : public AttributeDefImplWrap<BaseShapeAttributeType> {
	DECLARE_ATTRIBUTE_DEF_IMPL(BaseShapeAttributeType)

	AttributeDefImpl(){}
	virtual ~AttributeDefImpl(){}

	typedef std::shared_ptr<collision::BaseShape> Value;
	static Value deserialized(const AttributeParseInfo& a){ return a.object.getValue<Value>(); }
	static util::ObjectNode serialized(const Value& v){ return util::ObjectNode::create<Value>(v); }

	static Value initValue(){ return Value(); }
};

template<>
struct AttributeDefImpl<SignalArgumentAttributeType> : public AttributeDefImplWrap<SignalArgumentAttributeType> {
	DECLARE_ATTRIBUTE_DEF_IMPL(SignalArgumentAttributeType)

	AttributeDefImpl(){}
	virtual ~AttributeDefImpl(){}

	typedef nodes::SignalArgument Value;
	static Value deserialized(const AttributeParseInfo& a){ return a.object.getValue<Value>(); }
	static util::ObjectNode serialized(const Value& v){ return util::ObjectNode::create<Value>(v); }

	static Value initValue(){ return Value("", nodes::SignalType::Real); }

};

template<>
struct AttributeDefImpl<VertexAttributeType> : public AttributeDefImplWrap<VertexAttributeType> {
	DECLARE_ATTRIBUTE_DEF_IMPL(VertexAttributeType)

	AttributeDefImpl(){}
	virtual ~AttributeDefImpl(){}

	using Value= visual::Vertex;
	static Value deserialized(const AttributeParseInfo& a){ return a.object.getValue<Value>(); }
	static util::ObjectNode serialized(const Value& v){ return util::ObjectNode::create<Value>(v); }

	static Value initValue(){ return Value(); }

};

template<>
struct AttributeDefImpl<TriIndicesAttributeType> : public AttributeDefImplWrap<TriIndicesAttributeType> {
	DECLARE_ATTRIBUTE_DEF_IMPL(TriIndicesAttributeType)

	AttributeDefImpl(){}
	virtual ~AttributeDefImpl(){}

	using Value= util::DynArray<uint32>;
	static Value deserialized(const AttributeParseInfo& a);
	static util::ObjectNode serialized(const Value& v);

	static Value initValue(){ return Value(); }

};

GENERIC_ATTRIBUTE_DEF_IMPL(JointDefAttributeType, animation::JointDef);
GENERIC_ATTRIBUTE_DEF_IMPL(ArmatureAttachmentDefAttributeType, visual::ArmatureAttachmentDef);
GENERIC_ATTRIBUTE_DEF_IMPL(SrtTransform3AttributeType, util::SrtTransform3d);
GENERIC_ATTRIBUTE_DEF_IMPL(PhysObjectDefAttributeType, physics::EntityObjectDef);
GENERIC_ATTRIBUTE_DEF_IMPL(PhysJointDefAttributeType, physics::EntityJointDef);
GENERIC_ATTRIBUTE_DEF_IMPL(ClipChannelAttributeType, animation::ClipChannel);

/// Array
template <PlainAttributeType A>
struct AttributeDefImpl<AttributeType<A, true>> : public AttributeDefImplWrap<AttributeType<A, true>> {
	DECLARE_ATTRIBUTE_DEF_IMPL((AttributeType<A, true>))

	typedef AttributeDefImpl<AttributeType<A, false>> ElementDefType;

	AttributeDefImpl(const ElementDefType& def):
		elementDef(def){}

	virtual ~AttributeDefImpl(){}

	typedef util::DynArray<typename ElementDefType::Value> Value;
	const ElementDefType elementDef;

	static Value deserialized(const AttributeParseInfo& a);
	static util::ObjectNode serialized(const Value&);

	static Value initValue(){ return Value(); }
};


/// Editor creates gui components according to these
class AttributeDef {
public:

	AttributeDef(const BaseAttributeDefImpl&);

	/// "Constructors"
	static AttributeDef String(const util::Str8& key, const util::DynArray<util::Str8>& autofill= {}, uint32 maxlen=1000);
	static AttributeDef Resource(const util::Str8& key, const util::Str8& autosearch_tags = "");
	static AttributeDef ResourcePair(const util::Str8& key, const util::Str8& autosearch_tags = "");
	static AttributeDef Path(const util::Str8& key);
	static AttributeDef Real(const util::Str8& key, bool integer=false, real64 minvalue=-10000000, real64 maxvalue=10000000);
	static AttributeDef Vector2(const util::Str8& key);
	static AttributeDef Vector4(const util::Str8& key);
	static AttributeDef Boolean(const util::Str8& key);
	static AttributeDef Color(const util::Str8& key, bool has_alpha=true);
	static AttributeDef SignalArgument(const util::Str8& key);
	static AttributeDef SignalArgumentArray(const util::Str8& key);

	RuntimeAttributeType getType() const { return type; }
	const util::Str8& getKey() const { return impl->key; }

	template <typename T> // AttributeType
	const AttributeDefImpl<T>& getImpl() const {
		ensure_msg(T::runtimeType() == impl->getAttributeType(), "%i == %i", T::runtimeType(), impl->getAttributeType());
		return *static_cast<AttributeDefImpl<T>*>(impl.get());
	}

	AttributeDef(AttributeDef&&)= default;
	AttributeDef& operator=(AttributeDef&&)= default;
	AttributeDef(const AttributeDef&)= default;
	AttributeDef& operator=(const AttributeDef&)= default;

private:

	AttributeDef(const util::Str8& key_, BaseAttributeDefImpl* impl_);

	RuntimeAttributeType type;
	std::shared_ptr<BaseAttributeDefImpl> impl;
};

#include "attribute_def.tpp"

} // resources
} // clover

#endif // CLOVER_RESOURCES_ATTRIBUTE_DEF_HPP
