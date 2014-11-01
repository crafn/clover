#ifndef CLOVER_GAME_EDITOR_EDITORCOMPONENT_HPP
#define CLOVER_GAME_EDITOR_EDITORCOMPONENT_HPP

#include "build.hpp"
#include "global/eventreceiver.hpp"
#include "util/string.hpp"

// Add this to every inherited EditorComponent
#define DECLARE_EDITORCOMPONENT(type) public: virtual const util::Str8 getName() const { return EditorComponentTraits<type>::name(); }

namespace clover {
namespace game { namespace editor {

class EditorComponentFactory;

class EditorComponent : public global::EventReceiver {
public:
	EditorComponent();
	virtual ~EditorComponent();
	
	virtual void onEvent(global::Event&) override {}
	
	// Use DECLARE_EDITORCOMPONENT(EditorComponent) to implement this method
	virtual const util::Str8 getName() const = 0;
	
private:
	friend class EditorComponentFactory;
};

template <typename T>
struct EditorComponentTraits;// {
	//static const util::Str8 name(){ return "Default"; }
	
//};

}} // game::editor
} // clover

#endif // CLOVER_GAME_EDITOR_EDITORCOMPONENT_HPP