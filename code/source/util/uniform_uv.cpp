#include "uniform_uv.hpp"
#include "debug/print.hpp"

namespace clover {
namespace util {

util::Vec2f UniformUv::getUv(util::Vec2f pos) const {
	// @todo Rotation
	return (pos - uv.translation)*uv.scale.inversed();
}

} // utils
} // clover