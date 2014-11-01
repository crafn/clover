#ifndef CLOVER_GUI_LINEGRAPH_HPP
#define CLOVER_GUI_LINEGRAPH_HPP

#include "build.hpp"
#include "element_panel.hpp"
#include "element_textlabel.hpp"
#include "element_layout_grid.hpp"
#include "util/dyn_array.hpp"
#include "visual/entity_def_model.hpp"

namespace clover {
namespace gui {

/** @class GuiLineGraph A line graph gui component
	@brief Supports multiple lines and labels (if you need to use this, size and alignment-stuff need to be made automatic)
*/
class LineGraphElement : public PanelElement {
public:
	typedef PanelElement BaseClass;
	typedef uint32 NodeId;

	LineGraphElement(const util::Coord& offset, const util::Coord& radius);
	LineGraphElement(LineGraphElement&&)= delete;
	virtual ~LineGraphElement();

	NodeId createLine(const util::Color& c, const util::Str8& name);
	void appendPoint(const NodeId& id, const util::Vec2d& p);

	/// Sets area of which lines is shown. Vertical component is calculated automatically
	void setViewport(real64 left_x, real64 right_x);

	void clearBefore(real64 x);

	virtual void preUpdate();
	virtual void postUpdate();
	virtual void spatialUpdate() override;

private:
	class Line {
	public:
		Line(const util::Color& c);

		void append(const util::Vec2d& p);
		void clearBefore(real64 x);
		
		///	@return line segments within range
		visual::TriMesh generateMesh(real64 center_x, real64 radius_x);

	private:
		util::Color color;
		util::DynArray<util::Vec2f> points;
	};

	bool updateModel;

	real64 lineViewXAtCenter;
	util::Vec2d lineViewRadius;

	util::DynArray<Line> lines;
	util::DynArray<TextLabelElement> labels;
	gui::GridLayoutElement labelGrid;

	visual::TriMesh lineMesh;
	visual::Model lineModel;
	visual::ModelEntityDef modelEntityDef;
	CustomElementVisualEntity* visualEntity;

};

} // gui
} // clover

#endif // CLOVER_GUI_LINEGRAPH_HPP