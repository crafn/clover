#ifndef CLOVER_VISUAL_MODEL_TEXT_HPP
#define CLOVER_VISUAL_MODEL_TEXT_HPP

#include "build.hpp"
#include "util/coord.hpp"
#include "visual/font.hpp"
#include "visual/model.hpp"
#include "visual/mesh.hpp"

namespace clover {
namespace visual {

class TextModel : public Model {
public:
	TextModel();
	virtual ~TextModel();

	TextModel& operator=(const TextModel& re);
	TextModel& operator=(TextModel&&)= default;
	TextModel(const TextModel& re);
	TextModel(TextModel&&);

	void setActiveFont(const util::Str8& name);
	void setActiveFont(const util::Str8& name, Font::Size size);
	void setActiveFontSize(Font::Size size);
	
	/// @param relative_origo (0,0) == lower left, (1,1) upper right
	void setAlignment(util::Vec2d relative_origo);
	util::Vec2d getAlignment();

	util::Coord getDimensions() const;

	void setText(const util::Str8& s);
	void setText(const char8 *str, ...);

private:
	friend class EntityMgr;

	Font* activeFont;
	Font::Size activeSize;

	TriMesh textMesh;

	util::Vec2d relativeOrigo;
	util::Vec2d dimensions;
	util::Vec2d getTranslation();
};

} // visual
} // clover

#endif // CLOVER_VISUAL_MODEL_TEXT_HPP
