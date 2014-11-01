#ifndef CLOVER_GAME_EDITOR_EXTENSIONS_BLENDER_HPP
#define CLOVER_GAME_EDITOR_EXTENSIONS_BLENDER_HPP

#include "../editorextension.hpp"
#include "net/protocol_msg/server.hpp"
#include "build.hpp"
#include "global/eventreceiver.hpp"

namespace clover {
namespace game { namespace editor {

/// Blender EditorExtension
class BlenderEE : public EditorExtension, public global::EventReceiver {
public:
	BlenderEE();
	virtual ~BlenderEE(){}
	
	virtual void onEvent(global::Event& e) override;
	virtual void update() override;
	
private:
	net::msg::Server server;
};

}} // game::editor
} // clover

#endif // CLOVER_GAME_EDITOR_EXTENSIONS_BLENDER_HPP