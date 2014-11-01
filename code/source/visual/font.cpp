#include "font.hpp"
#include "global/exception.hpp"
#include "debug/debugprint.hpp"
#include "visual/bitmap.hpp"
#include "hardware/glstate.hpp"
#include "util/misc.hpp"
#include "util/string.hpp"

namespace clover {
namespace visual {

Font::GlyphInfo::GlyphInfo():
				advance(0){
}

const int32 Font::sizesToPixels[(int32)Size::Last]= {
		8,
		13,
		30,
		80
};

Font::Font(){
}

Font::~Font(){
}

void Font::create(FT_Face& face){
	FT_GlyphSlot slot= face->glyph;

	util::Vec2i mapsize {1024,1024};
	Bitmap bitmap(mapsize);

	util::Vec2i pen;
	int32 rowheight=0;

	glyphs.resize(glyphCountInSize*4+4);

	for (int32 a=0; a<(int32)Size::Last; a++){
		int32 pixsize=sizesToPixels[a];

		int32 error = FT_Set_Pixel_Sizes(face, 0, pixsize);

		if (error){
			throw global::Exception("Font::build(..): FT_Set_Pixel_Sizes fail");
		}

		for (FT_ULong i=0;; i++){

			FT_ULong ch=0;

			if (i < 0x007F){
				// Unicode Basic Latin
				ch= i;
			}
			else {
				// Latin-1 Supplement
				ch= 0x00A1 - 0x007F + i;
			}
			
			if (ch == 0x00FF+1){
				break;
			}
			
			error = FT_Load_Char(face, ch, FT_LOAD_RENDER);

			if (error)
				throw global::Exception("Font::create(..): FT_Load_Char(..) failed");

			if (pen.y >= mapsize.y) throw global::Exception("Font::create(..): too small texture");

			if (pen.x+slot->bitmap.width >= mapsize.x){
				pen.y += rowheight;
				pen.x= 0;
				rowheight=0;
			}
			
			GlyphInfo &glyphInfo= glyphs[ch + a*glyphCountInSize];

			glyphInfo.size= util::Vec2i{slot->bitmap.width, slot->bitmap.rows};
			glyphInfo.offset= util::Vec2i{int(slot->metrics.horiBearingX) >> 6, (int(slot->metrics.horiBearingY) >> 6) - slot->bitmap.rows};
			glyphInfo.advance= slot->advance.x >> 6;

			//print(debug::Ch::General, debug::Vb::Trivial, "advance %i width %i offset %i", glyphInfo.advance, glyphInfo.size.x, glyphInfo.offset.y);

			glyphInfo.uv1= util::Vec2f{ 	(real32)pen.x/mapsize.x,
									(real32)pen.y/mapsize.y};
			glyphInfo.uv2= glyphInfo.uv1 + util::Vec2f{ 	(real32)glyphInfo.size.x / mapsize.x,
													(real32)glyphInfo.size.y / mapsize.y};

			bitmap.blitBW(  slot->bitmap.buffer,
							glyphInfo.size,
							pen);

			pen.x += slot->bitmap.width + 1;
			if (slot->bitmap.rows > rowheight) rowheight= slot->bitmap.rows;
		}
	}
	
	tex= std::move(bitmap.createTexture());
	mat.setTexture(Material::TexType_Color, &tex);
}

TriMesh Font::generateMesh(Font::Size size, const util::Str8 str){
	TriMesh m;

	util::Vec2i pos;
	pos.y -= sizesToPixels[(int32)size];
	
	for (uint32 i=0; i<str.length(); i++){
		if (str[i] == 0)break;
		
		if (str[i] == '\n'){
			pos.y -= sizesToPixels[(int32)size];
			pos.x= 0;
			continue;
		}
		
		if (str[i] + (int32)size*glyphCountInSize >= glyphs.size())
			throw global::Exception("Font::generateMesh(..): glyph index out of range: %i, %i, %i", str[i] + (int32)size*glyphCountInSize, i, str[i]);
			
		GlyphInfo& glyph= glyphs[str[i] + (int32)size*glyphCountInSize];

		util::Vec2f left_corner, right_corner;
		left_corner= util::Vec2f{ (real32)(pos.x + glyph.offset.x), (real32)(pos.y + glyph.offset.y) };
		right_corner= util::Vec2f{ (real32)(left_corner.x + glyph.size.x), (real32)(left_corner.y + glyph.size.y) };

		m.addRectByCorners(left_corner, right_corner, glyph.uv1, glyph.uv2);
		pos.x += glyph.advance;
	}
	
	return m;
}

} // visual
} // clover