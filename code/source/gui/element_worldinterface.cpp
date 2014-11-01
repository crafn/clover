#include "element_worldinterface.hpp"

namespace clover {
namespace gui {

WorldInterfaceElement::WorldInterfaceElement():
		Element(util::Coord(0, util::Coord::View_Stretch), util::Coord(50, util::Coord::View_Stretch)){//Tarpeeks iso (100x100 unittia)
	elementType= WorldInterface;
}

WorldInterfaceElement::~WorldInterfaceElement(){
}

} // gui
} // clover