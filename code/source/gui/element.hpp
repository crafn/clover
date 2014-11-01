#ifndef CLOVER_GUI_ELEMENT_HPP
#define CLOVER_GUI_ELEMENT_HPP

#include "build.hpp"
#include "audio/audiosourcehandle.hpp"
#include "elementvisualentity.hpp"
#include "elementvisualentity_custom.hpp"
#include "elementvisualentity_text.hpp"
#include "hardware/device.hpp"
#include "gui/cursor.hpp"
#include "gui/callback.hpp"
#include "util/linkedlist.hpp"
#include "util/coord.hpp"
#include "util/math.hpp"
#include "util/ptrtable.hpp"
#include "util/time.hpp"

// Add this to every GuiElement-class
#define DECLARE_GUI_ELEMENT(x) typedef x This;

// Add this to define callback
#define GUI_CALLBACK(x) protected: Callback<This> x; public: void set ## x ## Callback(const CallbackFunction<This>& f){ x.assign(f); }

namespace clover {
namespace gui {

class GuiMgr;

/// Base class for gui elements
class Element {
public:

	DECLARE_GUI_ELEMENT(Element);

	enum ElementType {
		None,
		WorldInterface,
		Button,
		CheckBox,
		ComboBox,
		LinearLayout,
		GridLayout,
		RadialLayout,
		Panel,
		Slider,
		TextLabel,
		TextField,
		ListBox,
		WEIcon,
		AttributeField,
		ColorPicker,
		TextFieldArray,
		Type_Last
	};

	enum BoundingBoxState {
		Bb_None,
		Bb_Touch
	};

	Element(const util::Coord& offset = util::Coord(0, util::Coord::View_Fit), const util::Coord& rad = util::Coord(0, util::Coord::View_Fit));
	
	Element(const Element&)= delete;
	Element(Element&&);
	
	Element& operator=(const Element&)= delete;
	Element& operator=(Element&&);

	virtual ~Element();
	
	virtual void setOffset(const util::Coord& p);
	util::Coord getOffset() const { return offset; }
	
	void setPosition(const util::Coord& p);
	
	void setMinRadius(const util::Coord& r);
	util::Coord getMinRadius() const { return minRadius; }
	
	void setMaxRadius(const util::Coord& r);
	util::Coord getMaxRadius() const { return maxRadius; }
	
	util::Coord getPosition() const { return position; }
	
	virtual void setRadius(const util::Coord& r);
	util::Coord getRadius() const { return radius; }
	
	/// Can layouts stretch this element, or only scale
	/// Default: true
	void setStretchable(bool b= true){ stretchable= b; }
	bool isStretchable() const { return stretchable; }
	
	/// If true, element radius changes to minimum so that subElements fit inside (always between min and max though)
	/// Default: false
	void setAutoResize(bool b= true){ autoResize= b; }
	
	/// Minimizes radius taking subElements account
	/// Called in spatialUpdate if autoResize is true
	virtual void minimizeRadius();
	
	virtual void addSubElement(Element &element);
	virtual void removeSubElement(Element &element);

	util::DynArray<Element*> getSubElements() const { return subElements; }

	inline ElementType getType(){ return elementType; }
	inline Element* getSuperElement(){ return superElement; }
	
	bool isActive() const { return active; }
	virtual void setActive(bool b= true);
	
	/// Bounding box check
	bool isPointInside(util::Coord p);
	
	/// @todo Remove, replaced by spatialUpdate()
	virtual void preUpdate();

	/// @todo Rename to logicUpdate
	virtual void postUpdate();
	
	/// Called when position or scale is potentially changed
	virtual void spatialUpdate();
	void recursiveSpatialUpdate();
	
	/// Sets depths of visualEntities of this element and calls depthOrderUpdate for subElements
	/// One element uses one index for each visualEntity
	void depthOrderUpdate(int32& depth_index);
	
	void setTouchable(bool b=true){ touchable= b; }
	void setApplySuperActivation(bool b=true){ applySuperActivation= b; }

	/// GuiCursor calls
	virtual void onStopDragging();

	enum class DraggingType {
		Left,
		Middle,
		Right
	};
	
	/// RMB dragging
	void setDraggingType(DraggingType t){ draggingType= t; }
	
	/// Recursive, doesn't affect logic
	void setVisible(bool b= true);
	bool isVisible() const { return visible; }
	
	/// Recursive, disables user interaction and affects visuals
	void setEnabled(bool b= true);
	bool isEnabled() const { return enabled; }

	bool isTriggered() const { return triggered; }
	bool isTouched() const { return curBbState == Bb_Touch; }
	bool recursiveIsTouched() const;
	
	bool hasSubElement(const gui::Element& e) const;
	bool recursiveHasSubElement(const gui::Element& e) const;
	
	/// Brings element on top of other subElements in same superElement
	void bringTop();
	
	bool isDragged() const { return guiCursor->getDraggedElement() == this; }
	
	/// Used to show that something the gui is representing is seriously wrong
	/// Usually turns to red colour
	/// NOT to be used to indicate error with gui
	void setErrorVisuals(bool b= true);
	
	GUI_CALLBACK(OnTrigger) // Called when released with mouse or pressing hotkey
	GUI_CALLBACK(OnSecondaryTrigger) // RMB
	GUI_CALLBACK(OnDraggingStart)
	GUI_CALLBACK(OnDragging) // Called every frame when dragging is on
	GUI_CALLBACK(OnDraggingStop)
	
private:

	bool triggered;
	bool errorVisuals;
	
	void addSubElementImpl(Element& element);
	void removeSubElementImpl(Element& element);
	
protected:
	// Pointers to visual entities of this element
	util::DynArray<BaseElementVisualEntity*> visualEntities;
	
	// ElementVisualEntities are freed in BaseElementVisualEntity::commonUpdate
	ElementVisualEntity* createVisualEntity(ElementVisualEntity::Type t);
	TextElementVisualEntity* createTextVisualEntity(TextElementVisualEntity::Type t);
	CustomElementVisualEntity* createCustomVisualEntity();
	
	template <typename T>
	T* newVisualEntityConfig(T* e){ return static_cast<T*>(newVisualEntityConfigImpl(e)); }
	BaseElementVisualEntity* newVisualEntityConfigImpl(BaseElementVisualEntity* e);
	
	void destroyVisualEntity(BaseElementVisualEntity* entity);
	
	// Remember to call in destructor if entity is using def contained in the element,
	// because otherwise def will be destructed before entity leading to a dangling pointer
	// i.e. ugly hack
	void destroyVisuals();
	
	// Brings sub_element on top of other subElements
	void bringTop(Element& sub_element);
	
	ElementType elementType;
	
	bool active; // Most effective: if false, element is virtually nonexistent
	bool visible; // Only hides
	bool enabled; // Can user interact
	bool firstFrame;
	bool touchable;
	bool applySuperActivation;
	DraggingType draggingType;
	
	// If true, aspect ratio of radius is not necessarily preserved
	bool stretchable;
	
	// Adjust size by contents
	bool autoResize;
	
	int32 depthOffset;
	
	// Center
	util::Coord position;
	
	// Offset from super element
	util::Coord offset;
	
	util::Coord radius;

	util::Coord minRadius;
	util::Coord maxRadius;

	Element* superElement;

	BoundingBoxState curBbState, prevBbState;

	// Last element is on top
	util::DynArray<Element*> subElements;

	friend class GuiMgr;
	//Tähän tulee kaikki gui:n elementit
	static util::PtrTable<Element> elementTable;
	int32 tableIndex;
	
	
	static Cursor* guiCursor;
	static audio::AudioSourceHandle audioSource;

	void setSuperElement(Element*);
	
	// Sub element calls on super
	virtual void onSubDestroy(Element&);
	
	// Called when subElement is replaced by std::moving
	virtual void onSubSwap(Element& existing, Element& replacing);
};

} // gui
} // clover

#endif // CLOVER_GUI_ELEMENT_HPP