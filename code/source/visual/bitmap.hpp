#ifndef CLOVER_VISUAL_BITMAP_HPP
#define CLOVER_VISUAL_BITMAP_HPP

#include "build.hpp"
#include "util/math.hpp"
#include "visual/texture.hpp"

namespace clover {
namespace visual {

class Bitmap {
public:
	Bitmap(util::Vec2i size= util::Vec2i{0,0});
	virtual ~Bitmap();

	void create(util::Vec2i size);

	/// @param pos lower left corner
	/// @param font_conversion will make rect white*alpha
	void blitBW(const uint8 * rect, util::Vec2i size, util::Vec2i pos, bool font_conversion=true);

	Texture createTexture(bool font_mode=true);

	void destroy();

private:
	uint8 *buffer;
	int32 bufLen;

	util::Vec2i dim;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_BITMAP_HPP