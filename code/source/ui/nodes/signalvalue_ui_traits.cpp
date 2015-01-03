#include "animation/armature.hpp"
#include "resources/cache.hpp"
#include "resources/exception.hpp"
#include "nodes/nodeeventtype.hpp"
#include "signalvalue_ui_traits.hpp"

namespace clover {
namespace ui { namespace nodes {

auto SignalValueUiTraits<SignalType::EventType>::createViewElement() -> ViewElement{
	return (ViewElement("", util::Coord::VF(0)));
}

auto SignalValueUiTraits<SignalType::EventType>::createEditElement() -> EditElement{
	EditElement ret(util::Coord::P(0), util::Coord::P({200, 10}));
	
	updateList(ret);

	return (ret);
}

void SignalValueUiTraits<SignalType::EventType>::valueToView(const Value& v, ViewElement& e){
	e.setText(v);
}

void SignalValueUiTraits<SignalType::EventType>::valueToEdit(const Value& v, EditElement& e){
	const auto& types= global::g_env->resCache->getSubCache<NodeEventType>().getResources();
	
	for (SizeType i=0; i<types.size(); ++i){
		if (types[i]->getName() == v){
			e.setSelected(i);
			return;
		}
	}
	
}

auto SignalValueUiTraits<SignalType::EventType>::valueFromEdit(const EditElement& e) -> Value{
	return e.getSelectedText();
}

void SignalValueUiTraits<SignalType::EventType>::onEditActivationChange(EditElement& e){
	if (e.isActive())
		updateList(e); // Have to update because event types could have changed
}

void SignalValueUiTraits<SignalType::EventType>::clearView(ViewElement& e){
	e.setText("");
}

void SignalValueUiTraits<SignalType::EventType>::updateList(EditElement& e){
	util::Str8 selected;
	bool was_empty= e.empty();
	if (!was_empty)
		selected= e.getSelectedText();
		
	e.clear();
	const auto& types= global::g_env->resCache->getSubCache<NodeEventType>().getResources();
	for (auto& m : types){
		e.append(m->getName());
	}
	
	if (!was_empty)
		e.setSelected(selected);
}

auto SignalValueUiTraits<SignalType::ArmaturePose>::createViewElement()
-> ViewElement {
	return (ViewElement("", util::Coord::VF(0)));
}

auto SignalValueUiTraits<SignalType::ArmaturePose>::createEditElement()
-> EditElement {
	return (EditElement(gui::TextFieldElement::Type::Dev));
}

void SignalValueUiTraits<SignalType::ArmaturePose>::valueToView(const Value& v, ViewElement& e){
	if (v.isInvalid())
		e.setText("");
	else
		e.setText(v.getArmature().getName() + ".bind");
}

void SignalValueUiTraits<SignalType::ArmaturePose>::valueToEdit(const Value& v, EditElement& e){
	if (v.isInvalid())
		e.setText("");
	else
		e.setText(v.getArmature().getName());
}

auto SignalValueUiTraits<SignalType::ArmaturePose>::valueFromEdit(const EditElement& e)
-> Value {
	return global::g_env->resCache->getResource<animation::Armature>(e.getText()).getBindPose();
}

void SignalValueUiTraits<SignalType::ArmaturePose>::onEditActivationChange(EditElement& e){
	if (e.isActive()) e.setEditing();
}

void SignalValueUiTraits<SignalType::ArmaturePose>::clearView(ViewElement& e){
	e.setText("");
}

}} // ui::nodes
} // clover
