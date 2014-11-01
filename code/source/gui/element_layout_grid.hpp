#ifndef CLOVER_GUI_LAYOUT_GRID_HPP
#define CLOVER_GUI_LAYOUT_GRID_HPP

#include "build.hpp"
#include "element_layout.hpp"

namespace clover {
namespace gui {

class GridLayoutNode : public LayoutBaseNode<util::Vec2i> {
public:
	typedef LayoutBaseNode<util::Vec2i> BaseClass;

	GridLayoutNode(Element& e):BaseClass(e){}

	void setAlignment(util::Vec2d a){ alignment= a; }
	util::Vec2d getAlignment() const { return alignment; }

private:
	util::Vec2d alignment;
};

/// Sets subElements to a grid
/// @todo Automatic min-radius calculation
class GridLayoutElement : public LayoutElement<GridLayoutNode> {
public:
	typedef LayoutElement<GridLayoutNode> BaseType;

	GridLayoutElement(const util::Coord& offset, const util::Coord& rad, util::Vec2i grid_size= util::Vec2i(1));
	virtual ~GridLayoutElement();

	/// Sets relative minimum distance between edge and grid centers'
	/// @param m
	///		(0,0): No margin, (1,1): Same margin than distance between node centers
	void setMargin(const util::Vec2d& m);
	
	/// Sets grid cell count
	void setGridSize(const util::Vec2i& size);
	/// @return grid size (cell count)
	util::Vec2i getGridSize() const { return gridSize; }

	/// Sets subElements' alignment to this when inserted
	/// @param alignment
	///		(-1,-1): lower left corner of the subElement is on center
	///		(1,1): upper right corner of the subElement is on center
	void setDefaultAlignment(const util::Vec2d& alignment);
	
	bool isEmpty(const util::Vec2i& pos);
	
	virtual void addNode(Element& e, Node::PositionType node_pos);
	
	void preUpdate();
	
protected:
	void updateNodes();
	virtual void spatialUpdate() override;

	util::Vec2d relativeMargin;
	util::Vec2i gridSize;
	util::Vec2d defaultAlignment;
};

} // gui
} // clover

#endif // CLOVER_GUI_LAYOUT_GRID_HPP