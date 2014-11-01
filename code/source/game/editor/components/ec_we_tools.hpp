#ifndef CLOVER_GAME_EDITOR_COMPONENTS_EC_WE_TOOLS_HPP
#define CLOVER_GAME_EDITOR_COMPONENTS_EC_WE_TOOLS_HPP

#include "../editorcomponent.hpp"
#include "build.hpp"

namespace clover {
namespace game { namespace editor {

class WeToolsEc;

template <>
struct EditorComponentTraits<WeToolsEc> {
	static const util::Str8 name(){ return "WeTools"; }
};

class WeToolsEc : public EditorComponent {
public:
	DECLARE_EDITORCOMPONENT(WeToolsEc);
	
	WeToolsEc();
	virtual ~WeToolsEc(){}
	
	void spawn(uint32 name_id);
	void setDebugDrawActive(bool b= true);
	bool isDebugDrawActive() const;

	/// Controls saving/loading
	void setChunksLocked(bool b= true);
	bool isChunksLocked() const;
	
	util::DynArray<util::Str8> getWeNames() const;
	
	static void eraseTerrain(util::Vec2d pos);
	static void deleteWe(util::Vec2d pos);

};

}} // game::editor
} // clover

#endif // CLOVER_GAME_EDITOR_COMPONENTS_EC_WE_TOOLS_HPP