#ifndef CLOVER_GUI_ELEMENT_LISTVIEW_HPP
#define CLOVER_GUI_ELEMENT_LISTVIEW_HPP

#include "build.hpp"
#include "util/dyn_array.hpp"
#include "element.hpp"
#include "element_button.hpp"

namespace clover {
namespace gui {

class ButtonElement;
class ListViewNode;

class ListViewElement : public Element {
public:
	DECLARE_GUI_ELEMENT(ListViewElement)

	typedef Element BaseType;

	using Node= ListViewNode;
	using NodeId= SizeType;

	enum class NodeState {
		Enabled,
		Disabled,
		Error
	};

	enum class NodeType {
		Text
		//, CheckBox, RadioButton, Expand, ...
	};

	ListViewElement(const util::Coord& pos= util::Coord::P(0), const util::Coord& radius= util::Coord::P(0), const util::Coord& node_height= util::Coord::P(0));
	ListViewElement(ListViewElement&&)= default;
	virtual ~ListViewElement();

	virtual void setActive(bool b=true);

	virtual void preUpdate();
	virtual void postUpdate();

	NodeId append(const util::Str8& text, NodeType type= NodeType::Text, NodeState state= NodeState::Enabled);

	bool hasNode(const util::Str8& text) const;

	util::DynArray<NodeId> getSelected() const;
	NodeId getIndex(const util::Str8& text);

	void setSelected(NodeId id, bool s= true);
	void setSelected(const util::Str8& text, bool s= true);

	void clearSelection();

	NodeId getClosestZeroOffset() const;

	void snapTo(NodeId id);
	void snapTopMostTo(NodeId id);

	/// 0 to 1
	void setScroll(real64 s);
	real64 getScroll() const { return scroll; }

	void setWrapFade(bool b=true){ wrapFade= b; }

	const util::Str8& getText(NodeId id) const;

	uint32 size() const { return nodes.size(); }
	bool empty() const { return nodes.empty(); }

	void clear();

	virtual void setRadius(const util::Coord& r);
	void setNodeVerticalRadius(const util::Coord& r);

	void setSelectedHighlight(bool b){ highlightSelected= b; }

	util::Coord getNodeListHeight() const;

	GUI_CALLBACK(OnSelect)
	GUI_CALLBACK(OnOutTrigger)

private:
	static constexpr real64 gap= 3;
	void updateNodes();
	virtual void spatialUpdate() override;

	util::Coord nodeRadius;
	real64 scroll;
	bool wrapFade;
	bool highlightSelected;
	util::DynArray<ListViewNode> nodes;
};

class ListViewNode : public gui::ButtonElement {
public:
	ListViewNode(const util::Str8& text, const util::Coord& pos, const util::Coord& rad);
	ListViewNode(ListViewNode&&)= default;
	virtual ~ListViewNode();

	virtual void postUpdate();

	bool isSelected() const { return selected; }
	void setSelected(bool b=true){ selected= b; }

	void setLoopCount(int32 l){ loopCount= l; }
	int32 getLoopCount() const { return loopCount; }

	void setHighlight(bool b){ buttonVisual->setHighlightTarget(b); }

private:
	bool selected;
	int32 loopCount;
};

} // gui
} // clover

#endif // CLOVER_GUI_ELEMENT_LISTVIEW_HPP