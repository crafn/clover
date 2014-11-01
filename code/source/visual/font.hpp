#ifndef CLOVER_VISUAL_FONT_HPP
#define CLOVER_VISUAL_FONT_HPP

#include "build.hpp"
#include "util/math.hpp"
#include "visual/mesh.hpp"
#include "visual/texture.hpp"
#include "visual/material.hpp"

#include <wchar.h>
#include <ft2build.h>
#include <freetype.h>

namespace clover {
namespace visual {

class Font {
public:
	
	/// @todo Shouldn't be hard coded...
	enum class Size : int32 {
		Small,
		Normal,
		Large,
		Huge,
		Last
	};

	Font();
	Font(Font&&)= default;
	Font(const Font&)= delete;
	Font& operator=(const Font&)= delete;

	virtual ~Font();

	void create(FT_Face &face);

	/// 1 unit == 1 pixel
	/// Upper left corner is origo
	/// @param text UTF-8 encoded string
	TriMesh generateMesh(Size s, const util::Str8 text);

	const Texture& getTexture() const { return tex; }
	const Material& getMaterial() const { return mat; }
		Texture tex;
	Material mat;

private:
	struct GlyphInfo {
		GlyphInfo();

		util::Vec2i size;
		util::Vec2f uv1, uv2;
		util::Vec2i offset;
		int32 advance;
	};

	util::DynArray<GlyphInfo> glyphs;

	static const int32 glyphCountInSize= 0x00FF+1; // 0x00FF sisältää basic ja latin-1 supplementin
	static const int32 sizesToPixels[(int32)Size::Last];
};

} // visual
} // clover

#endif // CLOVER_VISUAL_FONT_HPP
