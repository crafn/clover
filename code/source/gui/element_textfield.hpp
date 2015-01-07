#ifndef CLOVER_GUI_TEXTFIELD_HPP
#define CLOVER_GUI_TEXTFIELD_HPP

#include "build.hpp"
#include "element.hpp"
#include "element_textlabel.hpp"
#include "ui/hid/taglistentry.hpp"
#include "util/optional.hpp"

#include <sstream>

namespace clover {
namespace gui {

class TextFieldElement : public Element {
public:
	DECLARE_GUI_ELEMENT(TextFieldElement)
	
	enum class Type {
		Dev,
		Menu
	};
	
	static util::Coord defaultRadius(Type t);
	
	//TextFieldElement(const util::Coord& position, const util::Coord& rad);
	TextFieldElement(Type type, const util::Coord& position= util::Coord::VF(0));
	TextFieldElement(TextFieldElement&&)= default;
	virtual ~TextFieldElement();
	
	TextFieldElement& operator=(TextFieldElement&&)= delete;
	
	virtual void setActive(bool b=true);

	virtual void postUpdate();
	
	void setCharacterWidth(int32 char_count);
	virtual void setRadius(const util::Coord& rad);
	
	void setText(const util::Str8& str);
	const util::Str8& getText() const { return getEntry().modified; }
	
	/// Sets only numbers allowed
	void setNumeric(bool b= true, bool allow_decimal = true);
	
	template <typename T>
	void setValueAs(const T& n);
	
	/// Returns error_value if fails
	template <typename T>
	T getValueAs(const T& error_value= 0) const;

	void setTextColorMul(const util::Color& c);
	
	void setEditing(bool b= true);
	bool isEditing() const { return editing; }
	
	void setHistorySize(SizeType s);

	GUI_CALLBACK(OnTextChange) // Called when text is set or modified by user
	GUI_CALLBACK(OnTextModify) // Called only when text is modified by user
	GUI_CALLBACK(OnEditingStateChange)
	GUI_CALLBACK(OnConfirm) // Called when enter is pressed when writing
	
private:
	void init();
	void createVisuals();
	
	virtual void spatialUpdate() override;
	
	ElementVisualEntity* bgVisual;
	TextLabelElement textLabel;
	
	bool editing;
	bool numeric;
	bool allowDecimal;
	
	struct HistoryEntry {
		util::Str8 modified;
		util::Str8 original;
	};
	
	HistoryEntry& getEntry(){ return history[currentHistoryEntryIndex]; }
	const HistoryEntry& getEntry() const { return history[currentHistoryEntryIndex]; }
	
	util::DynArray<HistoryEntry> history; // Index 0 is currently written text by default
	SizeType currentHistoryEntryIndex;
	
	util::Optional<ui::hid::TagListEntry> textInputTagEntry;
};

template <typename T>
void TextFieldElement::setValueAs(const T& v_){
	T v= v_;
	std::string str;
	std::stringstream ss;
	ss << v;
	ss >> str;
	
	getEntry().modified= util::Str8(str);
	textLabel.setText(getEntry().modified);
	OnTextChange(*this);
}

template <typename T>
T TextFieldElement::getValueAs(const T& error_value) const {
	T ret= error_value;
	std::stringstream ss;
	util::Str8 string= getEntry().modified.cStr();
	ss << string.cStr();
	ss >> ret;
	
	return ret;
}

} // gui
} // clover

#endif // CLOVER_GUI_TEXTFIELD_HPP
