#ifndef CLOVE_GAME_EDITOR_EC_PERFORMANCE_HPP
#define CLOVE_GAME_EDITOR_EC_PERFORMANCE_HPP

#include "../editorcomponent.hpp"
#include "build.hpp"

namespace clover {
namespace game { namespace editor {

class PerformanceEc;

template <>
struct EditorComponentTraits<PerformanceEc> {
	static const util::Str8 name(){ return "Performance"; }
};

class PerformanceEc : public EditorComponent {
public:
	DECLARE_EDITORCOMPONENT(PerformanceEc);
	virtual ~PerformanceEc(){}
};

}} // game::editor
} // clover

#endif // CLOVE_GAME_EDITOR_EC_PERFORMANCE_HPP