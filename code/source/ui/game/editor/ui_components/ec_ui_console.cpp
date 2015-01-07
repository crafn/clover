#include "ec_ui_console.hpp"
#include "resources/cache.hpp"
#include "visual/font.hpp"

namespace clover {
namespace ui { namespace game { namespace editor {

ConsoleEcUi::ConsoleEcUi(ConsoleEc& comp):
	BaseClass(comp),
	contentLayout(gui::LinearLayoutElement::Vertical, util::Coord::VF(0), getRadius()),
	controlLayout(gui::LinearLayoutElement::Horizontal, util::Coord::VF(0), util::Coord::VF({1,controlLayoutHeight})),
	filterButton("Filters", util::Coord::VF(0), util::Coord::VF({0.1,controlLayoutHeight})),
	commandField(gui::TextFieldElement::Type::Dev),
	filterFloating(util::Coord::VF(0)),
	filterPanel(util::Coord::VF(0), filterPanelRadius()),
	filterPanelLayout(gui::LinearLayoutElement::Vertical, util::Coord::VF(0), filterPanelRadius()),
	verbositySliderLabel("Verbosity",	util::Coord::VF(0)),
	verbositySlider(gui::SliderElement::Horizontal,	util::Coord::VF(0), util::Coord::VF(0.3)),
	filterLabel("debug::Print channels", util::Coord::VF(0)),
	filterLayout(util::Coord::VF(0), util::Coord::VF({0.25, 0.3}), {3, filterCheckCount/2+1}){

	contentLayout.setMargin(false);
	contentLayout.setMaxSpacing(util::Coord::VF(0));
	getContentElement().addSubElement(contentLayout);
	
	contentLayout.setStretchable();
	contentLayout.addNode(controlLayout, 999);
	contentLayout.mirrorFirstNodeSide();
	
	controlLayout.setStretchable();
	controlLayout.addNode(commandField);
	commandField.setStretchable();
	commandField.setHistorySize(100);
	controlLayout.addNode(filterButton);
	
	commandField.setOnConfirmCallback([&] (gui::TextFieldElement&){
		/// Execute console command
		getComponent().tryExecute(commandField.getText());
		commandField.setText("");
		commandField.setEditing();
	});
	
	filterButton.setOnTriggerCallback([&] (gui::Element&){
		filterFloating.setOffset(filterButton.getPosition());
		filterFloating.setActive();
	});
	
	filterFloating.addSubElement(filterPanel);
	filterFloating.setActive(false);
	filterFloating.setOnOutTriggerCallback([] (gui::FloatingElement& e){
		e.setActive(false);
	});
	
	filterPanel.addSubElement(filterPanelLayout);
	
	// Verbosity slider
	filterPanelLayout.addSubElement(verbositySliderLabel);
	filterPanelLayout.addSubElement(verbositySlider);
	verbositySlider.setStepCount(static_cast<int32>(debug::Vb::Last));
	verbositySlider.setOnValueModifyCallback([&] (gui::SliderElement& e){
		int32 value= e.getFixedValue();
		getComponent().setVerbosity((debug::Vb)value);
	});
	
	verbositySlider.setOnValueChangeCallback([&] (gui::SliderElement& e){
		util::Str8 value_str(debug::Print::getVerbosityString((debug::Vb)e.getFixedValue()));
		util::Str8 text= util::Str8("debug::Print verbosity: ") + value_str;

		verbositySliderLabel.setText(text);
	});

	// Channel filter checkboxes
	filterPanelLayout.addSubElement(filterLabel);
	filterPanelLayout.addSubElement(filterLayout);

	filterLayout.setDefaultAlignment({-1,0});
	filterLayout.setMargin({0,0.5});
	
	filterChecks.reserve(filterCheckCount);
	for (uint32 i=0; i<filterCheckCount; ++i){
		filterChecks.pushBack(std::move(
			gui::CheckBoxElement(debug::Print::getChannelString((debug::Ch)i), util::Coord::VF(0))));

		filterChecks.back().setOnValueModifyCallback([=] (gui::CheckBoxElement& e){
			getComponent().setChannelActive((debug::Ch)i, e.getValue());
		});

		int32 y= (i/2);
		y= filterCheckCount/2 - (1-filterCheckCount%2) - y;
		filterLayout.addNode(
			filterChecks.back(),
			{(int32)i%2, (int32)y});
	}
	
	onResize();
	readValues();
	
	listenForEvent(global::Event::OnConsoleBufferUpdate);
}

ConsoleEcUi::~ConsoleEcUi(){
}

void ConsoleEcUi::onEvent(global::Event& e){
	switch(e.getType()){
		case global::Event::OnConsoleBufferUpdate:
			append(*e(global::Event::Object).getPtr<debug::Print::Buffer>());
		break;
		
		default: break;
	}
}

void ConsoleEcUi::update(){
	readValues();
}

void ConsoleEcUi::onResize(){
	updateLineLabels();
	
	BaseClass::onResize();
	
	contentLayout.setRadius(getContentElement().getRadius());
	contentLayout.setMaxRadius(getContentElement().getRadius());
	
	controlLayout.setMaxRadius(controlLayout.getMaxRadius().onlyY() + getContentElement().getRadius().onlyX());
	controlLayout.setRadius(controlLayout.getMaxRadius());
	
	util::Coord r= controlLayout.getRadius().onlyX() - filterButton.getRadius().onlyX() 
								+ commandField.getMaxRadius().onlyY();
	if (r.x < 0) r.x= 0;
	commandField.setMaxRadius(r);
	//commandField.setRadius(r);
	
	updateLineLabels();
}

void ConsoleEcUi::append(const debug::Print::Buffer& buf){
	for (auto& msg : buf){
		
		auto separated_lines= util::Str8(msg.string.c_str()).splitted('\n');
		
		util::Color c({1.0, 1.0, 1.0, 0.8});
		
		if (msg.verbosity == debug::Vb::Moderate)
			c= util::Color({1.0, 1.0, 0.3, 0.8});
		else if (msg.verbosity == debug::Vb::Critical){
			c= util::Color({1.0, 0.2, 0.2, 0.8});
		}
		
		for (const auto& l : separated_lines){
			Line line;
			line.color= c;
			line.string= l;
			lines.push_back(std::move(line));
		}

	}

	while (lines.size() > 50){
		lines.pop_front();
	}
	
	updateLineLabels();
}

void ConsoleEcUi::updateLineLabels(){
	lineLabels.clear();
	lineLayouts.clear();
	
	util::Coord y_sum= util::Coord::VF(0);
	
	int order=0;
	for (auto it= lines.rbegin(); it != lines.rend(); ++it){
		
		auto& line= *it;
		
		gui::TextLabelElement label(util::Str8(line.string), util::Coord::VF(0));
		label.setColorMul(line.color);
		y_sum += label.getRadius().onlyY();
		
		if ((contentLayout.getRadius().onlyY() - y_sum.onlyY()).y <= filterButton.getRadius().y) break; // Out of space
		
		
		lineLabels.pushBack(std::move(label));
		
		lineLayouts.pushBack(std::move(gui::LinearLayoutElement(gui::LinearLayoutElement::Horizontal, util::Coord::VF(0), 
			contentLayout.getRadius().onlyX() + lineLabels.back().getRadius().onlyY())));
		lineLayouts.back().addSubElement(lineLabels.back());
		
		
		lineLayouts.back().setMargin(false);

		contentLayout.addNode(lineLayouts.back(), order);
		--order;
		
	}
	
	// Update positions and graphics manually to prevent blinking
	contentLayout.preUpdate();
}

void ConsoleEcUi::readValues(){
	if (filterFloating.isActive()){
		verbositySlider.setFixedValue(static_cast<int32>(getComponent().getVerbosity()));
		
		for (uint32 i=0; i<filterCheckCount; ++i){
			filterChecks[i].setValue(getComponent().isChannelActive((debug::Ch)i));
		}
	}
}

}}} // ui::game::editor
} // clover
