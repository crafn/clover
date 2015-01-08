#ifndef CLOVER_NODES_BASESLOT_HPP
#define CLOVER_NODES_BASESLOT_HPP

#include "build.hpp"
#include "signaltypetraits.hpp"

namespace clover {
namespace nodes {

class ENGINE_API BaseSlot {
public:
	BaseSlot(SignalType t);
	virtual ~BaseSlot();

	SignalType getType() const;
protected:
	SignalType type;
};

} // nodes
} // clover

#endif // CLOVER_NODES_BASESLOT_HPP
