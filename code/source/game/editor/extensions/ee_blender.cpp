#include "ee_blender.hpp"
#include "collision/shape.hpp"
#include "hardware/device.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace game { namespace editor {

using namespace net::msg;

struct CloverPathMsgTraits {
	NET_MSG_TRAITS_NAME("clpt") // CLover PaTh
	NET_MSG_TRAITS_VALUE(util::Str8) // Path to binary directory
};

struct ResourceRootMsgTraits {
	NET_MSG_TRAITS_NAME("rsrt") // ReSource RooT
	NET_MSG_TRAITS_VALUE(util::Str8) // Path from binary directory to resource root folder
};

struct ResourceMsgTraits {
	NET_MSG_TRAITS_NAME("rsrc") // ReSouRCe
	NET_MSG_TRAITS_VALUE(resources::SerializedResource)
};

struct ResourceRequestMsgTraits {
	NET_MSG_TRAITS_NAME("rsrq") // ReSource ReQuest
	NET_MSG_TRAITS_VALUE(resources::ResourceId)
};

BlenderEE::BlenderEE()
		: server(ServerInfo{"BlenderEE server", 19595, 5.0}){
	
	server.registerReceivable<ResourceMsgTraits>([=] (const resources::SerializedResource& res){
		print(debug::Ch::Net, debug::Vb::Trivial, "Resource received from Blender: %s", res.getTypeName().cStr());
		
		resources::gCache->parseResource(res);
	});
	
	server.registerReceivable<ResourceRequestMsgTraits>([=] (const resources::ResourceId& res_id){
		print(debug::Ch::Net, debug::Vb::Trivial, "Resource request from Blender: %s - %s", res_id.getTypeName().cStr(), res_id.getIdentifier().generateText().cStr());
		
		server.send<ResourceMsgTraits>(resources::gCache->getResource(res_id).getSerializedResource());
	});
	
	server.setOnConnectCallback([=] (){
		// Tell where clover is
		server.send<CloverPathMsgTraits>(hardware::gDevice->getWorkingDirectory());
		
		// Tell where resources are
		server.send<ResourceRootMsgTraits>(resources::gCache->getResourceRootPath());
	});
	
	listenForEvent(global::Event::OnEditorResourceSelect);
}

void BlenderEE::onEvent(global::Event& e){
	switch(e.getType()){
		case global::Event::OnEditorResourceSelect:
			if (server.isConnected()){
				// Resource selected, send to blender
				resources::Resource* res= e(global::Event::Resource).getPtr<resources::Resource>();
				ensure(res);
				
				server.send<ResourceMsgTraits>(res->getSerializedResource(), [=] (){
					print(debug::Ch::Dev, debug::Vb::Trivial, "Resource (%s) %s sent to Blender",
						res->getResourceTypeName().cStr(),
						res->getIdentifierAsString().cStr());
				});
			}
		break;
		
		default:;
	}
}

void BlenderEE::update(){
	server.update();
}

}} // game::editor
} // clover