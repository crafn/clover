#ifndef CLOVER_GUI_ATTRIBUTE_FIELD_HPP
#define CLOVER_GUI_ATTRIBUTE_FIELD_HPP

#include "build.hpp"
#include "element.hpp"
#include "element_textlabel.hpp"
#include "resources/attribute_def.hpp"

namespace clover {
namespace gui {

/// @todo Refactor to work with traits as SignalValueUi works, because
/// 	- custom gui class for every AttributeType is a bit overscaled
///		- redundant code is written when same kind of functionality is wanted somewhere else (like with signals)

/// @todo Should be moved to ui

/// Components that correspond to different AttributeDef-types are derived from this
class BaseAttributeFieldElement : public Element {
public:
	DECLARE_GUI_ELEMENT(BaseAttributeFieldElement)
	BaseAttributeFieldElement();
	virtual ~BaseAttributeFieldElement();
	
	virtual resources::RuntimeAttributeType getAttributeType() const = 0;
	virtual resources::PlainAttributeType getPlainAttributeType() const = 0;
	virtual void setLabel(const util::Str8& str) = 0;
	virtual util::Str8 getLabel() const = 0;
	virtual void setValue(const resources::BaseAttribute& attrib) = 0;
	virtual std::shared_ptr<resources::BaseAttribute> getValueAsAttribute() const = 0;
	virtual void onResize() = 0;
	
	/// Called when value is modified by user
	GUI_CALLBACK(OnValueModify)
	
private:
	
};

/// Derive actual specialized AttributeFieldElements from this
template <typename T> // AttributeType
class AttributeFieldWrapElement : public BaseAttributeFieldElement {
public:
	DECLARE_GUI_ELEMENT(AttributeFieldWrapElement)
	
	typedef BaseAttributeFieldElement BaseType;
	
	typedef resources::AttributeDefImpl<T> AttributeDefImplType;
	typedef typename AttributeDefImplType::Value Value;
	
	
	AttributeFieldWrapElement(const resources::AttributeDef& def);
	virtual ~AttributeFieldWrapElement();
	
	virtual resources::RuntimeAttributeType getAttributeType() const { return T::runtimeType(); }
	virtual resources::PlainAttributeType getPlainAttributeType() const { return T::plainAttributeType; }
	
	virtual void addSubElement(Element& e) override;

	virtual void setLabel(const util::Str8& str);
	virtual util::Str8 getLabel() const;
	
	void setValue(const Value& v);
	virtual void setValue(const resources::BaseAttribute& attrib);
	const Value& getValue() const { return value; }
	virtual std::shared_ptr<resources::BaseAttribute> getValueAsAttribute() const;
	
	const AttributeDefImplType& getAttributeDefImpl() const { return *attributeDefImpl; }
	
	virtual void onResize();
	
protected:
	void callOnValueModify(){ OnValueModify(*this); }
	
	virtual void onValueSet()= 0;
	
	/// Rules for attribute
	const AttributeDefImplType* attributeDefImpl;
	
	gui::TextLabelElement label;
	
	Value value;
	
	/// These are automatically resized to parent size
	/// @todo remove
	util::DynArray<Element*> resizeElements;
	
};

/// Specialize this for every resource::AttributeType
/// Remember to add its include to resources/attributetypes.def so AttributeFieldElementFactory works
/// Remember to call OnValueChange(*this) when attribute is changed
template <typename T>
class AttributeFieldElement : public AttributeFieldWrapElement<T> {
public:
	AttributeFieldElement(const resources::AttributeDef& def)
		: AttributeFieldWrapElement<T>(def){}
	virtual ~AttributeFieldElement(){}
	
protected:
	virtual void onValueSet(){}
};

#include "element_attributefield.tpp"

} // gui
} // clover
	
#endif // CLOVER_GUI_ATTRIBUTE_FIELD_HPP