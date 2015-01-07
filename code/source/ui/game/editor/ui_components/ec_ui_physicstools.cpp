#include "ec_ui_physicstools.hpp"
#include "util/map.hpp"

namespace clover {
namespace ui { namespace game { namespace editor {

PhysicsToolsEcUi::PhysicsToolsEcUi(PhysicsToolsEc& comp):
	EditorComponentUi(comp),
	contentLayout(gui::LinearLayoutElement::Horizontal, util::Coord::VF(0), util::Coord::VF(0)),
	primaryLayout(gui::LinearLayoutElement::Vertical, util::Coord::VF(0), util::Coord::VF({0.2, 0.3})),
		primaryTitleLabel("Primary mouse", util::Coord::VF(0)),
		primaryGrabLabel("Grab", util::Coord::VF(0)),
	secondaryLayout(gui::LinearLayoutElement::Vertical, util::Coord::VF(0), util::Coord::VF({0.2, 0.3})),
		secondaryTitleLabel("Secondary mouse", util::Coord::VF(0)),
		secondaryActionLabel("-", util::Coord::VF(0)),
	drawLayout(gui::LinearLayoutElement::Vertical, util::Coord::VF(0), util::Coord::VF({0.25, 0.3})),
	drawCheckLayout(gui::LinearLayoutElement::Horizontal, util::Coord::VSt(0), util::Coord::VF({0.25, 0.03})),
		drawCheck("Physics draw",				util::Coord::VF(0)),
		drawFlagCheckLayout({	gui::LinearLayoutElement(gui::LinearLayoutElement::Horizontal, util::Coord::VSt(0), util::Coord::VF({0.2,0.03})),
								gui::LinearLayoutElement(gui::LinearLayoutElement::Horizontal, util::Coord::VSt(0), util::Coord::VF({0.2,0.03})),
								gui::LinearLayoutElement(gui::LinearLayoutElement::Horizontal, util::Coord::VSt(0), util::Coord::VF({0.2,0.03})),
								gui::LinearLayoutElement(gui::LinearLayoutElement::Horizontal, util::Coord::VSt(0), util::Coord::VF({0.2,0.03})),
								gui::LinearLayoutElement(gui::LinearLayoutElement::Horizontal, util::Coord::VSt(0), util::Coord::VF({0.2,0.03})),
								gui::LinearLayoutElement(gui::LinearLayoutElement::Horizontal, util::Coord::VSt(0), util::Coord::VF({0.2,0.03})),
								gui::LinearLayoutElement(gui::LinearLayoutElement::Horizontal, util::Coord::VSt(0), util::Coord::VF({0.2,0.03}))}),
		drawFlagCheck({	gui::CheckBoxElement("Shape",			util::Coord::VF(0)),
						gui::CheckBoxElement("Joint",			util::Coord::VF(0)),
						gui::CheckBoxElement("Bounding box",	util::Coord::VF(0)),
						gui::CheckBoxElement("Broadphase pair",	util::Coord::VF(0)),
						gui::CheckBoxElement("Center of mass",	util::Coord::VF(0)),
						gui::CheckBoxElement("Fluid particles", util::Coord::VF(0)),
						gui::CheckBoxElement("Grid",			util::Coord::VF(0))}),
		drawAlphaSliderLabel("Alpha",					util::Coord::VF(0)),
		drawAlphaSlider(gui::SliderElement::Horizontal, util::Coord::VF(0),				util::Coord::VF({0.2})){
	
	EditorComponentUi::getContentElement().addSubElement(contentLayout);
	
	contentLayout.setSecondaryCentering();
	
	contentLayout.addSubElement(primaryLayout);
	contentLayout.addSubElement(secondaryLayout);
	contentLayout.addSubElement(drawLayout);
	
	//
	// PrimaryLayout
	//
	
	primaryLayout.setMaxSpacing(util::Coord::VF(0));
	primaryLayout.addSubElement(primaryTitleLabel);
	primaryLayout.addSubElement(primaryGrabLabel);
	
	//
	// SecondaryLayout
	//
	
	secondaryLayout.setMaxSpacing(util::Coord::VF(0));
	secondaryLayout.addSubElement(secondaryTitleLabel);
	secondaryLayout.addSubElement(secondaryActionLabel);
	
	//
	// DrawLayout
	//
	
	drawLayout.addSubElement(drawCheckLayout);

	// Physicsdraw checkbox
	drawCheckLayout.setMargin(false);
	drawCheckLayout.addSubElement(drawCheck);

	// Physicsdraw flags checkboxes
	for (uint32 i=0; i<drawFlagCheckCount; ++i){
		drawLayout.addSubElement(drawFlagCheckLayout[i]);
		drawFlagCheckLayout[i].setMargin(false);
		drawFlagCheckLayout[i].addSubElement(drawFlagCheck[i]);
		drawFlagCheck[i].setChecked(true);
	}

	// Physicsdraw alpha slider
	drawLayout.addSubElement(drawAlphaSliderLabel);
	drawLayout.addSubElement(drawAlphaSlider);
	
	drawCheck.setOnValueModifyCallback([&] (gui::CheckBoxElement& e){
		getComponent().setDrawingActive(e.isChecked());
	});
	
	for (uint32 i=0; i<drawFlagCheckCount; ++i){
		
		drawFlagCheck[i].setOnValueModifyCallback([=] (gui::CheckBoxElement& e){
			getComponent().setDrawingFlag(getComponent().getFlagEnum(i), e.isChecked());
			
			// Automatic enabling and disabling of draw
			if (e.getValue() == true)
				getComponent().setDrawingActive(true);
			else {
				bool found= false;
				for (uint32 j=0; j<drawFlagCheckCount; ++j){
					if (drawFlagCheck[j].isChecked()){
						found= true;
						break;
					}
				}
				if (!found)
					getComponent().setDrawingActive(false);
			}
		});
	}
		
	
	drawAlphaSlider.setOnValueModifyCallback([&] (gui::SliderElement& e){
		
		const real32 min= 0.02;
		real32 value= e.getValue()/(1.0 - min) + min;
		
		getComponent().setDrawingAlpha(value);
		
	});
	
	readValues();

#if 0

	if (infoPhysicsDrawCheck.isToggled()){
		if (infoPhysicsDrawCheck.isChecked()){
			global::g_env.debugDraw->setEnabled(debug::DebugDraw::DrawFlag::Physics);
		}
		else {
			global::g_env.debugDraw->setEnabled(debug::DebugDraw::DrawFlag::Physics, false);
		}
	}

	// Physics draw flags
	for (uint32 i=0; i<infoPhysicsDrawFlagCheckCount; ++i){
		if (infoPhysicsDrawFlagCheck[i].isToggled() || initFrame){

			bool state= infoPhysicsDrawFlagCheck[i].isChecked();
			physics::Draw::Flag f;

			switch(i){
				case 0: f= physics::Draw::Flag::BaseShape; break;
				case 1: f= physics::Draw::Flag::Joint; break;
				case 2: f= physics::Draw::Flag::BoundingBox; break;
				case 3: f= physics::Draw::Flag::BroadPhasePair; break;
				case 4: f= physics::Draw::Flag::CenterOfMass; break;
				default: ensure_msg(0, "Invalid index for enum");
			}

			global::g_env.debugDraw->getPhysicsDraw().setFlag(f, state);
		}
	}

#endif
	
}

void PhysicsToolsEcUi::update(){
	readValues();
	getComponent().update();
}

void PhysicsToolsEcUi::onResize(){
	BaseType::onResize();
	
	contentLayout.setMaxRadius(getContentElement().getRadius());
	contentLayout.setMinRadius(getContentElement().getRadius());
	contentLayout.setRadius(getContentElement().getRadius());
	
	if (contentLayout.getRadius().x < contentLayout.getRadius().y){
		contentLayout.setType(gui::LinearLayoutElement::Vertical);
	}
	else {
		contentLayout.setType(gui::LinearLayoutElement::Horizontal);
	}
}

void PhysicsToolsEcUi::readValues(){
	drawCheck.setChecked(getComponent().isDrawingActive());
	drawAlphaSlider.setValue(getComponent().getDrawingAlpha());
	
	for (uint32 i=0; i<drawFlagCheckCount; ++i){
		drawFlagCheck[i].setValue(getComponent().getDrawingFlag(getComponent().getFlagEnum(i)));
	}
	
	using JType= PhysicsToolsEc::JointType;
	static util::Map<JType, util::Str8> jointTypeNames= {
		{ JType::Distance, "Distance joint" },
		{ JType::Revolute, "Revolute joint" },
		{ JType::Prismatic, "Prismatic joint" },
		{ JType::Rope, "Rope joint" },
		{ JType::Weld, "Weld joint" },
		{ JType::HardWeld, "HardWeld joint" },
		{ JType::Wheel, "Wheel joint" },
		{ JType::Detach, "Detach" },
		{ JType::Mirror, "Mirror" }
	};

	secondaryActionLabel.setText(jointTypeNames[getComponent().getCurrentJointType()]);
}

}}} // ui::game::editor
} // clover
