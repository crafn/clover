#include "element_textfield.hpp"
#include "ui/userinput.hpp"

namespace clover {
namespace gui {

util::Coord TextFieldElement::defaultRadius(Type type){
	switch(type){
		default:
		case Type::Dev: return util::Coord::P({100,9});
		case Type::Menu: return util::Coord::P({100, 15});
	}
}

TextFieldElement::TextFieldElement(Type type, const util::Coord& position)
		: Element(position, util::Coord::P(0))
		, bgVisual(createVisualEntity(ElementVisualEntity::Type::Panel))
		, textLabel("TextField", util::Coord::VF(0))
		, editing(false)
		, numeric(false)
		, allowDecimal(true)
		, history(1)
		, currentHistoryEntryIndex(0){
	
	util::Coord min, rad, max;
	
	switch(type){
		case Type::Dev:
			min= util::Coord::P({10, 9});
			rad= defaultRadius(type);
			max= util::Coord::P({1000, 9});
			/// @todo Font
		break;
		
		case Type::Menu:
			min= util::Coord::P({10, 15});
			rad= defaultRadius(type);
			max= util::Coord::P({1000, 15});
			/// @todo Font
		break;
	}
		
	setMinRadius(min);
	setMaxRadius(max);
	setRadius(rad);
		
	init();
}

TextFieldElement::~TextFieldElement(){
}

void TextFieldElement::init(){
	elementType= TextField;
	createVisuals();
	addSubElement(textLabel);
}

void TextFieldElement::setActive(bool b){
	Element::setActive(b);
	
	if (!b && editing){
		editing= false;
		textInputTagEntry= boost::none;
	}
}

void TextFieldElement::postUpdate(){
	Element::postUpdate();
	
	if (!editing && curBbState == Bb_Touch && gUserInput->isTriggered(UserInput::GuiCause)){
		setEditing(true);
	}
	else if (editing && (
				(curBbState == Bb_None && gUserInput->isTriggered(UserInput::GuiCause)) ||
				(gUserInput->isTriggered(UserInput::GuiCancel)) ||
				(gUserInput->isTriggered(UserInput::GuiConfirm)))){
			
		setEditing(false);
		
		if (gUserInput->isTriggered(UserInput::GuiConfirm)){
			ensure(history.size() > 0);
			
			HistoryEntry e= getEntry();
			history[0]= e;
			history[0].original= history[0].modified; // Apply changes in entry
			
			if (currentHistoryEntryIndex > 0 && getEntry().modified == getEntry().original){
				// Using history entry
				// Move next entries towards past so that used entry is overwritten (= moved to beginning)
				for (SizeType i= currentHistoryEntryIndex; i > 0; --i)
					history[i]= history[i-1];
				
			}
			else {
				getEntry().modified= getEntry().original; // Reset modified history entry
				// Move history towards past (includes copying current text to history)
				for (SizeType i= history.size()-1; i > 0; --i)
					history[i]= history[i-1];
				
			}
			
			currentHistoryEntryIndex= 0;
			
			OnConfirm(*this);
		}
	}
	
	if (editing){
		bgVisual->setHighlightTarget(true);
		
		bool text_changed= false;
		
		// Going through history
		
		if (gUserInput->isTriggered(UserInput::GuiPrevHistoryEntry) && currentHistoryEntryIndex + 1 <= history.size()){
			++currentHistoryEntryIndex;
			text_changed= true;
		}
		
		if (gUserInput->isTriggered(UserInput::GuiNextHistoryEntry) && currentHistoryEntryIndex > 0){
			--currentHistoryEntryIndex;
			text_changed= true;
		}
		
		ensure(currentHistoryEntryIndex >= 0 && currentHistoryEntryIndex < history.size());
		
		// Writing
		util::Str8 written= gUserInput->getWrittenString();
		
		if (numeric){
			// Allow only numbers
			util::Str8 temp;
			for (uint32 i=0; i<written.length(); ++i){
				auto m= written[i];
				if (	(m >= '0' && m <= '9') || 
						(m == '.' && allowDecimal) || 
						m == '-' || 
						m == util::Str8::backspaceChar){
					temp += m;
				}
			}
			written= temp;
		}
		
		if (written.length() > 0){
			getEntry().modified= getEntry().modified + written;
			text_changed= true;
		}
		
		// Erasing
		uint32 bs= written.count(util::Str8::backspaceChar)*2; // *2 because backspace chars need to be erased too
		if (bs > 0 && getEntry().modified.length() > 0){
			int32 start = getEntry().modified.length()-bs;
			if (start < 0){
				start= 0;
				bs= getEntry().modified.length();
			}
			getEntry().modified.erase(start, bs);

			text_changed= true;
		}
		
		if (text_changed){
			textLabel.setText(getEntry().modified);
			
			OnTextChange(*this);
			OnTextModify(*this);
		}
		
	}
	else
		bgVisual->setHighlightTarget(false);
}

void TextFieldElement::setCharacterWidth(int32 char_count){
	util::Coord rad(getRadius().onlyY() + util::Coord::P({(real64)char_count*5, 0}));
	setRadius(rad); /// @todo Real character size
	setMinRadius(rad);
}

void TextFieldElement::setRadius(const util::Coord& rad){
	Element::setRadius(rad);
	
	//re->setScale(rad);
}

void TextFieldElement::setText(const util::Str8& str){
	getEntry().modified= str;
	textLabel.setText(getEntry().modified);
	
	OnTextChange(*this);
}

void TextFieldElement::setNumeric(bool b, bool allow_decimal){
	numeric= b;
	allowDecimal= allow_decimal;
}

void TextFieldElement::setTextColorMul(const util::Color& c){
	textLabel.setColorMul(c);
}

void TextFieldElement::setEditing(bool b){
	if (editing == b) return;
	
	if (b){
		editing= true;
		/// @todo Change to use channel of the guy who is editing
		textInputTagEntry= std::move(ui::hid::TagListEntry("host", "textInput"));
		
		OnEditingStateChange(*this);
	}
	else {
		editing= false;
		textInputTagEntry= boost::none;
		
		OnEditingStateChange(*this);
	}
}

void TextFieldElement::setHistorySize(SizeType s){
	history.resize(s+1); // See declaration of history
	
	if (currentHistoryEntryIndex > s){
		currentHistoryEntryIndex= s;
	}
}

void TextFieldElement::createVisuals(){
	//autoREs.resize(1);
	/*re= &autoREs[0];

	re->setModel("guiPanel");
	re->setLayer(visual::ModelEntity::Layer::Gui_Back);
	re->setEnvLight(0);
	re->setScale(radius);*/
}

void TextFieldElement::spatialUpdate(){
	Element::spatialUpdate();
	bgVisual->setPositionTarget(position);
	bgVisual->setRadiusTarget(radius);
}

} // gui
} // clover