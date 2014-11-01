#ifndef CLOVER_GUI_LAYOUT_HPP
#define CLOVER_GUI_LAYOUT_HPP

#include "build.hpp"
#include "element.hpp"

namespace clover {
namespace gui {

/// Base class for layout gui element
template <typename N>
class LayoutElement : public Element {
public:
	typedef N Node;

	LayoutElement(const util::Coord& offset, const util::Coord& rad):
			Element(offset, rad){
		touchable= false;
	}
	
	LayoutElement(LayoutElement&& other):
		Element(std::move(other)),
		nodes(std::move(other.nodes)){
		
		// Check for doubles
		for (auto it= nodes.begin(); it != nodes.end(); ++it){
			for (auto it2= it + 1; it2 != nodes.end(); ++it2){
				ensure(&it->getElement() != &it2->getElement());
			}
		}
	}
	
	LayoutElement(const LayoutElement& other)= delete;

	LayoutElement& operator=(LayoutElement&& other){
		Element::operator=(std::move(other));
		nodes= other.nodes;

		// Check for doubles
		for (auto it= nodes.begin(); it != nodes.end(); ++it){
			for (auto it2= it + 1; it2 != nodes.end(); ++it2){
				ensure(&it->getElement() != &it2->getElement());
			}
		}

		return *this;
	}

	LayoutElement& operator=(const LayoutElement&)= delete;

	virtual ~LayoutElement(){}
	
	virtual void addSubElement(Element& e){
		addNode(e);
	}
	
	Node& addNode(Element& e){
		Element::addSubElement(e);
		nodes.pushBack(Node(e));
		
		return nodes.back();
	}

	Node& addNode(Element& e, typename Node::PositionType node_position){
		addNode(e).setPosition(node_position);
		return nodes.back();
	}

	virtual void removeSubElement(Element& e){
		Element::removeSubElement(e);
		
		typename util::DynArray<Node>::Iter it= nodes.find(Node(e));
		ensure(it != nodes.end());
		nodes.erase(it);
	}
	
	virtual void setActive(bool b){
		Element::setActive(b);
	}
	
protected:
	virtual void onSubDestroy(Element& e){
		Element::onSubDestroy(e);
		for (auto it= nodes.begin(); it!= nodes.end(); ++it){
			if (&it->getElement() == &e){
				nodes.erase(it);
				break;
			}
		}
	}
	
	virtual void onSubSwap(Element& existing, Element& replacing){
		Element::onSubDestroy(existing);
		for (auto it= nodes.begin(); it!= nodes.end(); ++it){
			if (&it->getElement() == &existing){
				it->setElement(replacing);
				break;
			}
		}
		Element::addSubElement(replacing);
		//print(debug::Ch::Gui, debug::Vb::Trivial, "GuiLayout::onSubSwap(..): existing: %i, replacing: %i", &existing, &replacing);
	}
	
	util::DynArray<Node> nodes;
};

template <typename T>
class LayoutBaseNode {
public:
	typedef T PositionType;
	
	LayoutBaseNode(Element& e):
			element(&e),
			position(0){
	}
	
	LayoutBaseNode(const LayoutBaseNode& n):
			element(n.element),
			position(n.position){
	}
	
	LayoutBaseNode(LayoutBaseNode&& n):
			element(n.element),
			position(n.position){
	}
	
	LayoutBaseNode& operator=(const LayoutBaseNode& n){
		element= n.element;
		position= n.position;
		return *this;
	}

	LayoutBaseNode& operator=(LayoutBaseNode&&)= default;
	
	bool operator<(const LayoutBaseNode& n) const { return position < n.position; }
	bool operator==(const LayoutBaseNode& n) const { return element == n.element; }
	
	void setElement(Element& e){ element= &e; }
	Element& getElement(){ return *element; }
	const Element& getElement() const { return *element; }
	
	void setPosition(const T& p){ position= p; }
	T getPosition() const { return position; }
	
private:
	Element* element;
	T position;
};

} // gui
} // clover

#endif // CLOVER_GUI_LAYOUT_HPP