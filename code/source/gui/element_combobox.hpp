#ifndef CLOVER_GUI_COMBOBOX_HPP
#define CLOVER_GUI_COMBOBOX_HPP

#include "build.hpp"
#include "element.hpp"
#include "element_button.hpp"
#include "element_listview.hpp"
#include "ui/hid/taglistentry.hpp"
#include "util/optional.hpp"

namespace clover {
namespace gui {

class ComboBoxElement : public Element {
public:
	using Base= Element;
	DECLARE_GUI_ELEMENT(ComboBoxElement)

	ComboBoxElement(const util::Coord& pos= util::Coord::P(0), const util::Coord& max_rad= util::Coord::P(0));
	ComboBoxElement(ComboBoxElement&&)= default;
	virtual ~ComboBoxElement();
	
	uint32 append(const util::Str8& text);
	void clear();
	SizeType size(){ return listElement.size(); }
	
	virtual void setActive(bool b=true);
	
	virtual void preUpdate();
	virtual void postUpdate();
	
	uint32 getSelectedIndex() const;
	void setSelected(SizeType i);
	void setSelected(const util::Str8& text);
	
	/// Don't call if there's no nodes
	util::Str8 getSelectedText() const;
	
	void resetListPosition();
	
	void setListNodeVerticalRadius(const util::Coord& c);
	
	void setManualRadius(){
		autoRadius= false;
	}
	
	bool empty() const { return listElement.empty(); }
	
	GUI_CALLBACK(OnSelect) // Called when list closes by selecting item
	GUI_CALLBACK(OnSelectionChange)
	
private:
	void cancelList();
	virtual void spatialUpdate() override;
	static util::Coord defaultYRad(){ return util::Coord::P({0, 14}); }

	gui::ButtonElement btn;
	ListViewElement listElement;
	
	uint32 selectedIndex;
	real32 curY; 
	real32 stopY;
	bool autoRadius;
	
	ui::hid::ActionListener<nodes::SignalType::Vec2> scrollListener;
	util::Optional<ui::hid::TagListEntry> hoveringListSelectionTagEntry;
};

} // gui
} // clover

#endif // CLOVER_GUI_COMBOBOX_HPP
