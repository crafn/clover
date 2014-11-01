#ifndef CLOVER_UTIL_UNIFORM_UV_HPP
#define CLOVER_UTIL_UNIFORM_UV_HPP

#include "build.hpp"
#include "vector.hpp"
#include "transform.hpp"

namespace clover {
namespace util {

/// UV-information which helps setting uv coordinates of mesh
class UniformUv {
public:
	void setOrigo(util::Vec2f v){ uv.translation= v; }
	void setRotation(real32 r){ uv.rotation= r; }
	void setSize(util::Vec2f r){ uv.scale= r; };

	util::Vec2f getUv(util::Vec2f pos) const;
	
private:
	util::SrtTransform<util::Vec2f, real32, util::Vec2f> uv;
};

} // util
} // clover

#endif // CLOVER_UTIL_UNIFORM_UV_HPP