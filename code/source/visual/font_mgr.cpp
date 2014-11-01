#include "font_mgr.hpp"
#include "global/exception.hpp"
#include "debug/debugprint.hpp"
#include "util/string.hpp"

namespace clover {
namespace visual {

FontMgr* gFontMgr;

FontMgr::FontMgr(){
	int32 error= FT_Init_FreeType(&library);
	if (error)
		throw global::Exception("FontMgr::FontMgr(): FreeType init failed");
}

FontMgr::~FontMgr(){
	FT_Done_FreeType(library);
}

void FontMgr::createFont(const util::Str8& path, const util::Str8& name, int32 face_index){
	FT_Face face;
	int32 error= FT_New_Face( library,
							path.cStr(),
							face_index,
							&face);

	if (error == FT_Err_Unknown_File_Format)
		throw global::Exception("FontMgr::createFont(..): font %s is unsupported type", path.cStr());
	else if (error)
		throw global::Exception("FontMgr::createFont(..): couldn't load font %s", path.cStr());

	Font& f= fonts[name];
	f.create(face);
	FT_Done_Face(face);
}

Font &FontMgr::getFont(const util::Str8& name){
	auto it= fonts.find(name);
	if (it == fonts.end()){
		release_ensure(!fonts.empty());
		return fonts.begin()->second;
	}
	return fonts[name];
}

} // visual
} // clover