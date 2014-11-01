#ifndef CLOVER_GUI_LAYOUT_RADIAL_HPP
#define CLOVER_GUI_LAYOUT_RADIAL_HPP

#include "build.hpp"
#include "element_layout.hpp"

namespace clover {
namespace gui {

class RadialLayoutElement : public LayoutElement<LayoutBaseNode<int32>> {
	using LayoutElement::Node;
	typedef LayoutElement<LayoutBaseNode<int32>> BaseType;
public:
	RadialLayoutElement(const util::Coord& pos, const util::Coord& rad);
	RadialLayoutElement(RadialLayoutElement&&)= default;
	RadialLayoutElement(const RadialLayoutElement&)= delete;
	virtual ~RadialLayoutElement();
	
	virtual void preUpdate();
	
private:
	void updateNodes();
	virtual void spatialUpdate() override;
};

} // gui
} // clover

#endif // CLOVER_GUI_LAYOUT_RADIAL_HPP