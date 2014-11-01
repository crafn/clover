#ifndef CLOVER_NODES_SIGNALTYPE_HPP
#define CLOVER_NODES_SIGNALTYPE_HPP

#include "build.hpp"

namespace clover {
namespace nodes {

enum class SignalType : int32 {
	None = -1,
	#define SIGNAL(x, n) x = n,
	#include "signaltypes.def"
	#undef SIGNAL
};

enum class SubSignalType {
	None = -1,
	#define SUBSIGNAL(x, n) x = n,
	#include "subsignaltypes.def"
	#undef SUBSIGNAL
};

} // nodes
} // clover

#endif // CLOVER_NODES_SIGNALTYPE_HPP