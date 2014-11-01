#ifndef CLOVER_GUI_WORLDINTERFACE_HPP
#define CLOVER_GUI_WORLDINTERFACE_HPP

#include "build.hpp"
#include "element.hpp"

namespace clover {
namespace gui {
	
/// @todo This shuoldn't be part of gui
class WorldInterfaceElement : public Element {
public:
	WorldInterfaceElement();
	virtual ~WorldInterfaceElement();
};

} // gui
} // clover

#endif // CLOVER_GUI_WORLDINTERFACE_HPP
