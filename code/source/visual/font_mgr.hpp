#ifndef CLOVER_VISUAL_FONT_MGR_HPP
#define CLOVER_VISUAL_FONT_MGR_HPP

#include "build.hpp"
#include "visual/font.hpp"
#include "util/hashmap.hpp"

namespace clover {
namespace visual {

/// @todo Should be resource cache
class FontMgr {
public:
	FontMgr();
	virtual ~FontMgr();
	
	void createFont(const util::Str8& path, const util::Str8& name, int32 face_index=0);
	Font &getFont(const util::Str8& name);
	
private:
	FT_Library	library;
	util::HashMap<util::Str8, Font> fonts;
};

extern FontMgr* gFontMgr;

} // visual
} // clover

#endif // CLOVER_VISUAL_FONT_MGR_HPP
