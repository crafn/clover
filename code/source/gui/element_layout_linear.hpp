#ifndef CLOVER_GUI_LAYOUT_LINEAR_HPP
#define CLOVER_GUI_LAYOUT_LINEAR_HPP

#include "build.hpp"
#include "element_layout.hpp"

namespace clover {
namespace gui {

class LinearLayoutNode : public LayoutBaseNode<int32> {
public:
	LinearLayoutNode(Element& e):
		LayoutBaseNode<int32>(e){
	}
	
	LinearLayoutNode(LinearLayoutNode&&)= default;
	LinearLayoutNode(const LinearLayoutNode&)= default;
	virtual ~LinearLayoutNode(){}
	
	LinearLayoutNode& operator=(const LinearLayoutNode&)= default;
	LinearLayoutNode& operator=(LinearLayoutNode&&)= default;
	
	/// If set to true, element is stretched to match the size of layout (util::limited by min and max radius of element)
	/// If false, element is scaled to fit in the layout
	/// Default: true
	bool isStretchable() const { return getElement().isStretchable(); }
	
private:
};

/// Aligns subelements vertically/horizontally
class LinearLayoutElement : public LayoutElement<LinearLayoutNode> {
	using LayoutElement::Node;
	typedef LayoutElement<LinearLayoutNode> BaseType;
public:
	enum Type {
		Horizontal,
		Vertical
	};

	LinearLayoutElement(Type t, const util::Coord& offset= util::Coord::VF(0), const util::Coord& rad= util::Coord::VF(0));
	LinearLayoutElement(LinearLayoutElement&&)= default;
	LinearLayoutElement& operator=(LinearLayoutElement&&)= default;
	virtual ~LinearLayoutElement();

	void setType(Type t){ type= t; }
	
	/// False by default
	void setSecondaryCentering(bool b=true){ centerSecondary= b; }
	
	/// Should borders have similar gap as elements (default=true)
	void setMargin(bool b=true){ margin= b; }

	void setMinSpacing(const util::Coord& min_spacing);
	
	void setMaxSpacing(const util::Coord& max_spacing);
	
	void mirrorFirstNodeSide(bool b=true){ firstNodeSideMirrored= b; }

	/// Sets radius & maxRadius to smallest possible value depending on subElements (and minRadius)
	virtual void minimizeRadius() override;
	
	/// Default: true
	/// Not well tested
	void setAutomaticMinRadius(bool b=true){ automaticMinRadius= b; }
	
	/// Behaves like inactive subElements doesn't exist
	/// Default: false
	void setIgnoreInactive(bool b=true){ ignoreInactive= b; }
	
	void preUpdate();
	
	Type getLayoutType() const { return type; }
	
	virtual void spatialUpdate() override;
	
protected:
	util::Coord getSecLimitedRadius(Element& e);
	util::Coord calcMinimumRadius() const;
	void updateNodes();

	Type type;

	util::Coord minSpacing;
	util::Coord maxSpacing;
	bool centerSecondary;
	bool margin;
	bool firstNodeSideMirrored;
	bool automaticMinRadius;
	bool ignoreInactive;
};

} // gui
} // clover

#endif // CLOVER_GUI_LAYOUT_LINEAR_HPP