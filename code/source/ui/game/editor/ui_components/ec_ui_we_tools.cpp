#include "ec_ui_we_tools.hpp"

namespace clover {
namespace ui { namespace game { namespace editor {

WeToolsEcUi::WeToolsEcUi(WeToolsEc& comp):
	EditorComponentUi(comp),
	contentLayout(gui::LinearLayoutElement::Horizontal, util::Coord::VF(0), util::Coord::VF(0)),
	spawnLayout(gui::LinearLayoutElement::Vertical, util::Coord::VF(0), util::Coord::VF({0.3, 0.3})),
		spawnTitleLabel("Spawn", util::Coord::VF(0)),
		spawnCombo(util::Coord::VF(0), util::Coord::VF({0.3,0.03})),
		spawnButton("Spawn", util::Coord::VF(0), util::Coord::VF({0.1,0.03})),
	debugLayout(gui::LinearLayoutElement::Vertical, util::Coord::VF(0), util::Coord::VF({0.3, 0.3})),
		debugTitleLabel("Debug", util::Coord::VF(0)),
		debugDrawCheckLayout(gui::LinearLayoutElement::Horizontal, util::Coord::VSt(0), util::Coord::VF({0.3, 0.03})),
		debugDrawCheck("DebugDraw", util::Coord::VSt(0)),
		chunksLockedCheck("Lock chunks", util::Coord::VSt(0)){
	
	EditorComponentUi::getContentElement().addSubElement(contentLayout);
	
	contentLayout.setSecondaryCentering();
	
	contentLayout.addSubElement(spawnLayout);
	contentLayout.addSubElement(debugLayout);
	
	//
	// SpawnLayout
	//
	
	spawnLayout.setMaxSpacing(util::Coord::VF(0.01));
	spawnLayout.addSubElement(spawnTitleLabel);
	spawnLayout.addSubElement(spawnCombo);
	spawnCombo.setManualRadius();
	spawnLayout.addSubElement(spawnButton);
	
	auto names= getComponent().getWeNames();
	for (auto& m : names){
		spawnCombo.append(m);
	}
	
	spawnButton.setOnTriggerCallback([&] (gui::Element& e){
		getComponent().spawn(spawnCombo.getSelectedIndex());
	});
	
	//
	// DebugLayout
	//
	
	debugLayout.setMaxSpacing(util::Coord::VF(0.01));
	debugLayout.addSubElement(debugTitleLabel);
	debugLayout.addSubElement(debugDrawCheckLayout);
	debugLayout.addSubElement(chunksLockedCheck);

	chunksLockedCheck.setOnValueModifyCallback([&] (gui::CheckBoxElement&e ){
		getComponent().setChunksLocked(e.getValue());
	});
	
	debugDrawCheckLayout.addSubElement(debugDrawCheck);
	
	debugDrawCheck.setOnValueModifyCallback([&] (gui::CheckBoxElement& e){
		getComponent().setDebugDrawActive(e.getValue());
	});
	
	//
	// Shortcuts
	//
	
	terrainEraseListener= ui::hid::ActionListener<>("host", "dev", "eraseTerrain", [this] (){
		util::Vec2d cursor_on_world= gUserInput->getCursorPosition().converted(util::Coord::World).getValue();
		getComponent().eraseTerrain(cursor_on_world);
		
	});
	
	weDeleteListener= ui::hid::ActionListener<>("host", "dev", "deleteWe", [this] (){
		util::Vec2d cursor_on_world= gUserInput->getCursorPosition().converted(util::Coord::World).getValue();
		getComponent().deleteWe(cursor_on_world);
	});
}

void WeToolsEcUi::update(){
	readValues();
}

void WeToolsEcUi::onResize(){
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

void WeToolsEcUi::readValues(){
	debugDrawCheck.setValue(getComponent().isDebugDrawActive());
	chunksLockedCheck.setValue(getComponent().isChunksLocked());
}

}}} // ui::game::editor
} // clover