#ifndef CLOVER_UI_NODES_SIGNALVALUE_UI_TRAITS_HPP
#define CLOVER_UI_NODES_SIGNALVALUE_UI_TRAITS_HPP

#include "build.hpp"
#include "nodes/signaltypetraits.hpp"
#include "gui/element_checkbox.hpp"
#include "gui/element_combobox.hpp"
#include "gui/element_textfield.hpp"
#include "gui/element_textfieldarray.hpp"
#include "gui/element_textlabel.hpp"

namespace clover {
namespace ui { namespace nodes {
using namespace clover::nodes;

template <SignalType S>
struct SignalValueUiTraits {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::TextLabelElement EditElement; /// Can't really edit this
	typedef typename SignalTypeTraits<S>::Value Value;

	static ViewElement createViewElement(){ return (ViewElement("", util::Coord::VF(0))); }
	static EditElement createEditElement(){ return (ViewElement("", util::Coord::VF(0))); }
	static void valueToView(const Value& v, ViewElement& e){}
	static void valueToEdit(const Value& v, EditElement& e){}
	static Value valueFromEdit(const EditElement& e){ return SignalTypeTraits<S>::defaultInitValue(); }
	static void onEditActivationChange(EditElement& e){}
	static void clearView(ViewElement& e){ e.setText(""); }
};

template <>
struct SignalValueUiTraits<SignalType::Real> {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::TextFieldElement EditElement;
	typedef SignalTypeTraits<SignalType::Real>::Value Value;

	static ViewElement createViewElement(){
		return (ViewElement("", util::Coord::VF(0)));
	}

	static EditElement createEditElement(){
		gui::TextFieldElement e(gui::TextFieldElement::Type::Dev);
		e.setCharacterWidth(15);
		e.setValueAs(0.0);
		e.setNumeric();

		return (e);
	}

	static void valueToView(const Value& v, ViewElement& e){
		e.setAsNumeric(v);
	}

	static void valueToEdit(const Value& v, EditElement& e){
		e.setValueAs(v);
	}

	static Value valueFromEdit(const EditElement& e){
		return e.getValueAs<Value>();
	}

	static void onEditActivationChange(EditElement& e){
		if (e.isActive()) e.setEditing();
	}

	static void clearView(ViewElement& e){
		e.setText("");
	}
};

template <>
struct SignalValueUiTraits<SignalType::Integer> {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::TextFieldElement EditElement;
	typedef SignalTypeTraits<SignalType::Integer>::Value Value;

	static ViewElement createViewElement(){
		return (ViewElement("", util::Coord::VF(0)));
	}

	static EditElement createEditElement(){
		gui::TextFieldElement e(gui::TextFieldElement::Type::Dev);
		e.setCharacterWidth(15);
		e.setValueAs(0);
		e.setNumeric(true, true);

		return (e);
	}

	static void valueToView(const Value& v, ViewElement& e){
		e.setAsNumeric(v);
	}

	static void valueToEdit(const Value& v, EditElement& e){
		e.setValueAs(v);
	}

	static Value valueFromEdit(const EditElement& e){
		return e.getValueAs<Value>();
	}

	static void onEditActivationChange(EditElement& e){
	}

	static void clearView(ViewElement& e){
		e.setText("");
	}
};

template <>
struct SignalValueUiTraits<SignalType::String> {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::TextFieldElement EditElement;
	typedef SignalTypeTraits<SignalType::String>::Value Value;

	static ViewElement createViewElement(){
		return (ViewElement("", util::Coord::VF(0)));
	}

	static EditElement createEditElement(){
		return (EditElement(gui::TextFieldElement::Type::Dev));
	}

	static void valueToView(const Value& v, ViewElement& e){
		e.setText(v);
	}

	static void valueToEdit(const Value& v, EditElement& e){
		e.setText(v);
	}

	static Value valueFromEdit(const EditElement& e){
		return e.getText();
	}

	static void onEditActivationChange(EditElement& e){
		if (e.isActive()) e.setEditing();
	}

	static void clearView(ViewElement& e){
		e.setText("");
	}
};

template <>
struct SignalValueUiTraits<SignalType::Boolean> {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::CheckBoxElement EditElement;
	typedef SignalTypeTraits<SignalType::Boolean>::Value Value;

	static ViewElement createViewElement(){
		return (ViewElement("", util::Coord::VF(0)));
	}

	static EditElement createEditElement(){
		return (EditElement("", util::Coord::VF(0)));
	}

	static void valueToView(const Value& v, ViewElement& e){
		if (v) e.setText("True");
		else e.setText("False");
	}

	static void valueToEdit(const Value& v, EditElement& e){
		e.setChecked(v);
	}

	static Value valueFromEdit(const EditElement& e){
		return e.isChecked();
	}

	static void onEditActivationChange(EditElement& e){
	}

	static void clearView(ViewElement& e){
		e.setText("");
	}
};


template <>
struct SignalValueUiTraits<SignalType::Vec2> {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::TextFieldArrayElement EditElement;
	typedef SignalTypeTraits<SignalType::Vec2>::Value Value;

	static ViewElement createViewElement(){
		return (ViewElement("", util::Coord::VF(0)));
	}

	static EditElement createEditElement(){
		gui::TextFieldArrayElement e(2, util::Coord::VF(0), util::Coord::VF({0.15,0.03}));
		e[0].setNumeric();
		e[1].setNumeric();

		return (e);
	}

	static void valueToView(const Value& v, ViewElement& e){
		e.setText(util::Str8::format("%f, %f", v.x, v.y));
	}

	static void valueToEdit(const Value& v, EditElement& e){
		e[0].setValueAs(v.x);
		e[1].setValueAs(v.y);
	}

	static Value valueFromEdit(const EditElement& e){
		return Value { e[0].getValueAs<Value::Value>(), e[1].getValueAs<Value::Value>()};
	}

	static void onEditActivationChange(EditElement& e){
	}

	static void clearView(ViewElement& e){
		e.setText("");
	}
};

template <>
struct SignalValueUiTraits<SignalType::Vec3> {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::TextFieldArrayElement EditElement;
	typedef SignalTypeTraits<SignalType::Vec3>::Value Value;

	static ViewElement createViewElement(){
		return (ViewElement("", util::Coord::VF(0)));
	}
	
	static EditElement createEditElement(){
		gui::TextFieldArrayElement e(3, util::Coord::VF(0), util::Coord::VF({0.15,0.03}));
		e[0].setNumeric();
		e[1].setNumeric();
		e[2].setNumeric();

		return (e);
	}
	
	static void valueToView(const Value& v, ViewElement& e){
		e.setText(util::Str8::format("%f, %f, %f", v.x, v.y, v.z));
	}
	
	static void valueToEdit(const Value& v, EditElement& e){
		e[0].setValueAs(v.x);
		e[1].setValueAs(v.y);
		e[2].setValueAs(v.z);
	}
	
	static Value valueFromEdit(const EditElement& e){
		return Value { e[0].getValueAs<Value::Value>(), e[1].getValueAs<Value::Value>(), e[2].getValueAs<Value::Value>()};
	}
	
	static void onEditActivationChange(EditElement& e){}
	static void clearView(ViewElement& e){
		e.setText("");
	}
};


template <>
struct SignalValueUiTraits<SignalType::Quaternion> {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::TextFieldArrayElement EditElement;
	typedef SignalTypeTraits<SignalType::Quaternion>::Value Value;

	static ViewElement createViewElement(){
		return (ViewElement("", util::Coord::VF(0)));
	}

	static EditElement createEditElement(){
		gui::TextFieldArrayElement e(4, util::Coord::VF(0), util::Coord::VF({0.4,0.03}));
		for (auto& m : e)
			m.setNumeric();
		return e;
	}

	static void valueToView(const Value& v, ViewElement& e){
		std::stringstream ss;
		ss << v.axis().x << ", " << v.axis().y << ", " << v.axis().z << " ; " << v.rotation();
		e.setText(util::Str8(ss.str()));
	}

	static void valueToEdit(const Value& v, EditElement& e){
		Value::Vec axis= v.axis();
		e[0].setValueAs(v.x);
		e[1].setValueAs(v.y);
		e[2].setValueAs(v.z);
		e[3].setValueAs(v.rotation());
	}

	static Value valueFromEdit(const EditElement& e){
		return Value(	Value::byRotationAxis(
							util::Vec3d{	e[0].getValueAs<Value::Value>(),
									e[1].getValueAs<Value::Value>(),
									e[2].getValueAs<Value::Value>() },
									e[3].getValueAs<Value::Value>()
						)
					);
	}

	static void onEditActivationChange(EditElement& e){}
	static void clearView(ViewElement& e){
		e.setText("");
	}
};

template <>
struct SignalValueUiTraits<SignalType::RtTransform2> {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::TextFieldArrayElement EditElement;
	typedef SignalTypeTraits<SignalType::RtTransform2>::Value Value;

	static ViewElement createViewElement(){
		return (ViewElement("", util::Coord::VF(0)));
	}

	static EditElement createEditElement(){
		gui::TextFieldArrayElement e(3, util::Coord::VF(0), util::Coord::VF({0.2,0.03}));
		for (auto& m : e)
			m.setNumeric();

		return (e);
	}

	static void valueToView(const Value& v, ViewElement& e){
		std::stringstream ss;
		ss <<  v.rotation << " ; " << v.translation.x << ", " << v.translation.y;
		e.setText(util::Str8(ss.str()));
	}

	static void valueToEdit(const Value& v, EditElement& e){
		e[0].setValueAs(v.rotation);
		e[1].setValueAs(v.translation.x);
		e[2].setValueAs(v.translation.y);
	}

	static Value valueFromEdit(const EditElement& e){
		return Value(	e[0].getValueAs<Value::Rotation>(),
						{e[1].getValueAs<Value::Translation::Value>(),
						e[2].getValueAs<Value::Translation::Value>()});
	}

	static void onEditActivationChange(EditElement& e){
	}

	static void clearView(ViewElement& e){
		e.setText("");
	}
};

template <>
struct SignalValueUiTraits<SignalType::SrtTransform2> {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::TextFieldArrayElement EditElement;
	typedef SignalTypeTraits<SignalType::SrtTransform2>::Value Value;

	static ViewElement createViewElement(){
		return (ViewElement("", util::Coord::VF(0)));
	}

	static EditElement createEditElement(){
		gui::TextFieldArrayElement e(5, util::Coord::VF(0), util::Coord::VF({0.5,0.03}));
		for (auto& m : e)
			m.setNumeric();

		return (e);
	}

	static void valueToView(const Value& v, ViewElement& e){
		std::stringstream ss;
		ss << v.scale.x << ", " << v.scale.y << " ; " << v.rotation << " ; " << v.translation.x << ", " << v.translation.y;
		e.setText(util::Str8(ss.str()));
	}

	static void valueToEdit(const Value& v, EditElement& e){
		e[0].setValueAs(v.scale.x);
		e[1].setValueAs(v.scale.y);
		e[2].setValueAs(v.rotation);
		e[3].setValueAs(v.translation.x);
		e[4].setValueAs(v.translation.y);
	}

	static Value valueFromEdit(const EditElement& e){
		return Value(		{	e[0].getValueAs<Value::Scale::Value>(),
								e[1].getValueAs<Value::Scale::Value>()	},
								e[2].getValueAs<Value::Rotation>(),
							{	e[3].getValueAs<Value::Translation::Value>(),
								e[4].getValueAs<Value::Translation::Value>()	});
	}

	static void onEditActivationChange(EditElement& e){
	}

	static void clearView(ViewElement& e){
		e.setText("");
	}
};

template <>
struct SignalValueUiTraits<SignalType::SrtTransform3> {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::TextFieldArrayElement EditElement;
	typedef SignalTypeTraits<SignalType::SrtTransform3>::Value Value;

	static ViewElement createViewElement(){
		return (ViewElement("", util::Coord::VF(0)));
	}

	static EditElement createEditElement(){
		gui::TextFieldArrayElement e(10, util::Coord::VF(0), util::Coord::VF({0.8,0.03}));
		for (auto& m : e)
			m.setNumeric();
		return e;
	}

	static void valueToView(const Value& v, ViewElement& e){
		std::stringstream ss;
		ss	<< v.scale.x << ", " << v.scale.y << ", " << v.scale.z << " ; "
			<< v.rotation.axis().x <<  ", " << v.rotation.axis().y << ", " << v.rotation.axis().z << ", " << v.rotation.rotation() << " ; "
			<< v.translation.x << ", " << v.translation.y << ", " << v.translation.z;
		e.setText(util::Str8(ss.str()));
	}

	static void valueToEdit(const Value& v, EditElement& e){
		e[0].setValueAs(v.scale.x);
		e[1].setValueAs(v.scale.y);
		e[2].setValueAs(v.scale.z);
		e[3].setValueAs(v.rotation.axis().x);
		e[4].setValueAs(v.rotation.axis().y);
		e[5].setValueAs(v.rotation.axis().z);
		e[6].setValueAs(v.rotation.rotation());
		e[7].setValueAs(v.translation.x);
		e[8].setValueAs(v.translation.y);
		e[9].setValueAs(v.translation.z);
	}

	static Value valueFromEdit(const EditElement& e){
		return Value(		{	e[0].getValueAs<Value::Scale::Value>(), // Scale
								e[1].getValueAs<Value::Scale::Value>(),
								e[2].getValueAs<Value::Scale::Value>()	},
							Value::Rotation::byRotationAxis(
									{	e[3].getValueAs<Value::Rotation::Value>(), // Quaternion axis
										e[4].getValueAs<Value::Rotation::Value>(),
										e[5].getValueAs<Value::Rotation::Value>()	},
									e[6].getValueAs<Value::Rotation::Value>()), // Quaternion rotation
							{	e[7].getValueAs<Value::Translation::Value>(), // Translation
								e[8].getValueAs<Value::Translation::Value>(),
								e[9].getValueAs<Value::Translation::Value>()	});
	}

	static void onEditActivationChange(EditElement& e){}
	static void clearView(ViewElement& e){
		e.setText("");
	}
};

template <>
struct SignalValueUiTraits<SignalType::Event> {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::TextLabelElement EditElement; /// Can't really edit this
	typedef SignalTypeTraits<SignalType::Event>::Value Value;

	static ViewElement createViewElement(){ return (ViewElement("", util::Coord::VF(0))); }
	static EditElement createEditElement(){ return (ViewElement("", util::Coord::VF(0))); }
	static void valueToView(const Value& v, ViewElement& e){}
	static void valueToEdit(const Value& v, EditElement& e){}
	static Value valueFromEdit(const EditElement& e){ return Value(); }
	static void onEditActivationChange(EditElement& e){}
	static void clearView(ViewElement& e){ e.setText(""); }
};

template <>
struct SignalValueUiTraits<SignalType::EventType> {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::ComboBoxElement EditElement;
	typedef SignalTypeTraits<SignalType::EventType>::Value Value;

	static ViewElement createViewElement();
	static EditElement createEditElement();
	static void valueToView(const Value& v, ViewElement& e);
	static void valueToEdit(const Value& v, EditElement& e);
	static Value valueFromEdit(const EditElement& e);
	static void onEditActivationChange(EditElement& e);
	static void clearView(ViewElement& e);
	
private:
	static void updateList(EditElement& e);
};

template <>
struct SignalValueUiTraits<SignalType::Trigger> {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::TextLabelElement EditElement;
	typedef SignalTypeTraits<SignalType::Trigger>::Value Value;

	static ViewElement createViewElement(){ return (ViewElement("", util::Coord::VF(0))); }
	static EditElement createEditElement(){ return (EditElement("", util::Coord::VF(0))); }
	static void valueToView(const Value& v, ViewElement& e){ e.setText("*"); }
	static void valueToEdit(const Value& v, EditElement& e){}
	static Value valueFromEdit(const EditElement& e){ return Value(); }
	static void onEditActivationChange(EditElement& e){}
	static void clearView(ViewElement& e){ e.setText(""); }
};

template <>
struct SignalValueUiTraits<SignalType::ArmaturePose> {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::TextFieldElement EditElement;
	typedef SignalTypeTraits<SignalType::ArmaturePose>::Value Value;

	static ViewElement createViewElement();
	static EditElement createEditElement();
	static void valueToView(const Value& v, ViewElement& e);
	static void valueToEdit(const Value& v, EditElement& e);
	static Value valueFromEdit(const EditElement& e);
	static void onEditActivationChange(EditElement& e);
	static void clearView(ViewElement& e);
};

/// Default ui traits for a SignalType with ResourceRef<Res> value
template <typename Res>
struct SignalValueUiTraitsResourceRefMixIn {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::TextFieldElement EditElement; /// @todo Support for a non-string identifier
	typedef resources::ResourceRef<Res> Value;

	static ViewElement createViewElement(){ return ViewElement("", util::Coord::VF(0)); }
	static EditElement createEditElement(){ return EditElement(gui::TextFieldElement::Type::Dev); }
	static void valueToView(const Value& v, ViewElement& e){ e.setText(v.get().getIdentifier()); }
	static void valueToEdit(const Value& v, EditElement& e){ e.setText(v.get().getIdentifier()); }
	static Value valueFromEdit(const EditElement& e){ return e.getText(); }
	static void onEditActivationChange(EditElement& e){ if (e.isActive()) e.setEditing(); }
	static void clearView(ViewElement& e){ e.setText(""); }
};

template <>
struct SignalValueUiTraits<SignalType::Shape> : public SignalValueUiTraitsResourceRefMixIn<collision::Shape> {};

template <>
struct SignalValueUiTraits<SignalType::Vec4> {
	typedef gui::TextLabelElement ViewElement;
	typedef gui::TextFieldArrayElement EditElement;
	typedef SignalTypeTraits<SignalType::Vec4>::Value Value;

	static ViewElement createViewElement(){
		return (ViewElement("", util::Coord::VF(0)));
	}
	
	static EditElement createEditElement(){
		gui::TextFieldArrayElement e(4, util::Coord::VF(0), util::Coord::VF({0.2,0.03}));
		e[0].setNumeric();
		e[1].setNumeric();
		e[2].setNumeric();
		e[3].setNumeric();

		return (e);
	}
	
	static void valueToView(const Value& v, ViewElement& e){
		e.setText(util::Str8::format("%f, %f, %f, %f", v.x, v.y, v.z, v.w));
	}
	
	static void valueToEdit(const Value& v, EditElement& e){
		e[0].setValueAs(v.x);
		e[1].setValueAs(v.y);
		e[2].setValueAs(v.z);
		e[3].setValueAs(v.w);
	}
	
	static Value valueFromEdit(const EditElement& e){
		return Value {	e[0].getValueAs<Value::Value>(), e[1].getValueAs<Value::Value>(),
						e[2].getValueAs<Value::Value>(), e[3].getValueAs<Value::Value>()};
	}
	
	static void onEditActivationChange(EditElement& e){}
	static void clearView(ViewElement& e){
		e.setText("");
	}
};

}} // ui::nodes
} // clover

#endif // CLOVER_UI_NODES_SIGNALVALUE_UI_TRAITS_HPP
