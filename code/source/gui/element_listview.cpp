#include "element_listview.hpp"
#include "hardware/mouse.hpp"
#include "ui/userinput.hpp"

namespace clover {
namespace gui {

ListViewElement::ListViewElement(const util::Coord& pos, const util::Coord& rad, const util::Coord& node_height):
		Element(pos, rad),
		scroll(0),
		wrapFade(true),
		highlightSelected(true){
	
	depthOffset= 1000;
	
	nodeRadius= util::Coord({0, node_height.y}, node_height.getType()) + util::Coord({rad.x, 0}, rad.getType());
	nodeRadius= nodeRadius*util::Vec2d{0.9,1.0};
	
	setMinRadius(util::Coord::P({3,3}));
}

ListViewElement::~ListViewElement(){
}

void ListViewElement::setActive(bool b){
	Element::setActive(b);
}

void ListViewElement::preUpdate(){
	Element::preUpdate();
}

void ListViewElement::postUpdate(){
	Element::postUpdate();
	
	if (!isActive()) return;
	for (auto& m : nodes){
		if (m.isTriggered()){
			/// Only one selection is possible for now
			for (auto& m2 : nodes)
				m2.setSelected(false);
				
			m.setSelected();
			
			OnSelect(*this);
		}
	}
	
	if ((	hardware::gMouse->isPressed(GLFW_MOUSE_BUTTON_LEFT) ||
			hardware::gMouse->isPressed(GLFW_MOUSE_BUTTON_RIGHT) ||
			hardware::gMouse->isPressed(GLFW_MOUSE_BUTTON_MIDDLE)) && 
		!recursiveIsTouched()){
			
		OnOutTrigger(*this);
	}
}

auto ListViewElement::append(const util::Str8& text, NodeType type, NodeState state) -> NodeId {
	nodes.pushBack(std::move(ListViewNode(text, position, nodeRadius)));
	
	if (state == NodeState::Error){
		nodes.back().setErrorVisuals(true);
	}
	else if (state == NodeState::Disabled){
		nodes.back().setEnabled(false);
	}
	
	addSubElement(nodes.back());
	
	// nodeRadius can't be smaller than minimum size of nodes
	for (SizeType i= 0; i < nodes.size(); ++i){
		const util::Coord& rad1= nodeRadius.converted(util::Coord::View_Pixels);
		const util::Coord& rad2= nodes[i].getMinRadius().converted(util::Coord::View_Pixels) + util::Coord::P({10, 2});
		if (rad1.getValue().x < rad2.getValue().x){
			nodeRadius= nodeRadius.onlyY() + nodes[i].getMinRadius().onlyX();
		}
		if (rad1.getValue().y < rad2.getValue().y){
			nodeRadius= nodeRadius.onlyX() + nodes[i].getMinRadius().onlyY();
		}
	}
	
	setMinRadius(nodeRadius.onlyX() + getNodeListHeight().onlyY()*0.5);
	setRadius(nodeRadius.onlyX() + getNodeListHeight().onlyY()*0.5); // Just lazy
	setMaxRadius(nodeRadius.onlyX() + getNodeListHeight().onlyY()*0.5); // Just lazy
	
	if (isActive()){
		updateNodes();
		
		nodes.back().setActive(); // No fade
		
		for (auto& m : nodes) // preUpdate so positions are updated
			m.preUpdate();
	}
	
	return nodes.size()-1;
}

void ListViewElement::setSelected(NodeId id, bool s){
	ensure(!nodes.empty());
	ensure(id < nodes.size());
	nodes[id].setSelected(s);
}

void ListViewElement::setSelected(const util::Str8& text, bool s){
	ensure(hasNode(text));
	setSelected(getIndex(text), s);
}

void ListViewElement::clearSelection(){
	for (SizeType i=0; i<nodes.size(); ++i){
		nodes[i].setSelected(false);
	}
}

auto ListViewElement::getClosestZeroOffset() const -> NodeId {
	int32 closest=-1;
	real32 closest_dist=0;
	for (uint32 i=0; i<nodes.size(); ++i){
		
		real32 dist= util::abs(nodes[i].getOffset().y);
		
		if ( closest == -1 || (closest_dist > dist) ){
			closest= i;
			closest_dist= dist;
		}
	}
	return closest;
}

void ListViewElement::snapTo(NodeId id){
	ensure(!nodes.empty());
	scroll= (real64)id/nodes.size();
	
}

void ListViewElement::snapTopMostTo(NodeId id){
	ensure(!nodes.empty());
	util::Coord::Type type= nodeRadius.getType();
	util::Coord y_for_topmost= radius.converted(type).y - nodeRadius.y;
	
	if (getNodeListHeight().converted(radius.getType()).y < radius.y*2){
		y_for_topmost= getNodeListHeight().converted(type).y/2;
	}
	
	scroll= (real64)id/nodes.size() + y_for_topmost.y/getNodeListHeight().converted(type).y;
}

void ListViewElement::setScroll(real64 s){
	scroll= s;
	updateNodes();
}

bool ListViewElement::hasNode(const util::Str8& text) const {
	for (const auto& m : nodes){
		if (m.getText() == text)
			return true;
	}
	
	return false;
}

auto ListViewElement::getSelected() const -> util::DynArray<NodeId> {
	util::DynArray<NodeId> a;
	for (uint32 i=0; i<nodes.size(); ++i){
		if (nodes[i].isSelected()){
			a.pushBack(i);
		}
	}
	return a;
}

ListViewElement::NodeId ListViewElement::getIndex(const util::Str8& text){
	for (NodeId i= 0; i < (NodeId)nodes.size(); ++i){
		if (nodes[i].getText() == text)
			return i;
	}
	
	throw global::Exception("ListViewElement::getIndex(..): Node not found: %s", text.cStr());
}

const util::Str8& ListViewElement::getText(NodeId id) const {
	ensure(!nodes.empty());
	return nodes[id%nodes.size()].getText();
}

void ListViewElement::clear(){
	nodes.clear();
	ensure(subElements.empty());
}

void ListViewElement::setRadius(const util::Coord& r){
	Element::setRadius(r);
	
	nodeRadius= util::Coord({0, nodeRadius.y}, nodeRadius.getType()) + util::Coord({r.x, 0}, r.getType());
}

void ListViewElement::setNodeVerticalRadius(const util::Coord& r){
	nodeRadius= nodeRadius.onlyX() + r.onlyY();
}

void ListViewElement::updateNodes(){
	util::Coord::Type type= nodeRadius.getType();
	
	bool active= isActive();
	
	real64 totalheight= getNodeListHeight().converted(type).y;
	
	util::Coord node_pos= util::Coord({0, scroll*totalheight}, type);
	
	real64 upper_limit= totalheight*0.5;
	real64 lower_limit= -totalheight*0.5;
	
	for (uint32 i=0; i<nodes.size(); ++i){
		
		real64 total_pos= (node_pos).converted(type).y;
		int32 loop_count=0;
		
		/// @todo Detect going outside screen
		while (total_pos > upper_limit || total_pos < lower_limit){
			
			if (total_pos < lower_limit){
				
				++loop_count;
				
				node_pos.y += totalheight;
				total_pos += totalheight;
			}
			else if (total_pos > upper_limit){
				
				--loop_count;
				node_pos.y -= totalheight;
				total_pos -= totalheight;
			}
		
		}
	
		int32 last_loop_count= nodes[i].getLoopCount();
		
		ensure(std::isfinite(node_pos.x) && std::isfinite(node_pos.y));
		nodes[i].setOffset(node_pos);
		nodes[i].setLoopCount(loop_count);
		nodes[i].setRadius(nodeRadius);
		
		// Distance from center
		real32 dist= util::abs(total_pos);
		
		real32 dist_from_edge= util::abs(dist - totalheight*0.5f);
		
		real32 alpha= util::limited(dist_from_edge*10, 0.0f, 1.0f);
		if (dist > radius.y)
			alpha= 0;
			
		if (!wrapFade)
			alpha= 1;
		
		//if (active)
		//	nodes[i].setAlpha(alpha);
		
		if (highlightSelected)
			nodes[i].setHighlight(nodes[i].isSelected());
		
		node_pos -= util::Coord({0,nodeRadius.y*2}, nodeRadius.getType());
		node_pos -= util::Coord::P({0, gap});
	}
}

void ListViewElement::spatialUpdate(){
	BaseType::spatialUpdate();
	updateNodes();
}

util::Coord ListViewElement::getNodeListHeight() const {
	return util::Coord((nodes.size())*(nodeRadius.y)*2, nodeRadius.getType()) + util::Coord::P({0,gap*(nodes.size())});
}

ListViewNode::ListViewNode(const util::Str8& text_, const util::Coord& pos, const util::Coord& rad):
		ButtonElement(text_, pos, rad),
		selected(false),
		loopCount(0){
}


ListViewNode::~ListViewNode(){
}

void ListViewNode::postUpdate(){
	Element::postUpdate();
	
	if (selected){
		//re->setEnvLight(0.5);
	}
	else {
		//re->setEnvLight(0);
	}
}

} // gui
} // clover