#include "server.hpp"

namespace clover {
namespace net { namespace msg {

Server::Server(const ServerInfo& info_)
		: info(info_){
}

void Server::update(){
	try {
		ioService.poll();
		
		if (!connectionPtr && !listenerPtr){ // Nobody's listening
			startListening();
		}
		else if (connectionPtr && listenerPtr && connectionPtr->isConnected()){ // Just connected, stop listening
			print(debug::Ch::Net, debug::Vb::Trivial, "%s: Connected", info.name.cStr());
			stopListening();
		}
		else if (connectionPtr && !listenerPtr){ // Connected to blender
			if (connectionPtr && connectionPtr->isConnected()){
				
				if (connectionPtr->getTimeFromLastContact() > info.timeoutDuration)
					throw NetException("Timeout, dropping client");
				
			}
			else {
				throw NetException("Connection interrupted");
			}
		}
		else {
			//print(debug::Ch::Net, debug::Vb::Trivial, "%s: Waiting for connection... %i, %i", info.name.cStr(), connectionPtr.get() != nullptr, listenerPtr.get() != nullptr);
		}
		
	}
	catch (const boost::exception& e){
		print(debug::Ch::Net, debug::Vb::Critical, "%s: Network error", info.name.cStr());
		disconnect();
		stopListening();
	}
	catch (const std::exception& e){
		print(debug::Ch::Net, debug::Vb::Critical, "%s: Network error: %s", info.name.cStr(), e.what());
		disconnect();
		stopListening();
	}
}

void Server::startListening(){
	print(debug::Ch::Net, debug::Vb::Moderate, "%s: Listening started", info.name.cStr());
	
	ioService.reset();
	listenerPtr= Listener::create(ioService, info.port);
	connectionPtr= listenerPtr->listen(std::bind(&Server::onConnect, this));

	for (auto& m : recvCallbacks){
		connectionPtr->registerReceivable(m.name, m.callback);
	}
}

void Server::stopListening(){
	print(debug::Ch::Net, debug::Vb::Trivial, "%s: Listening stopped", info.name.cStr());
	listenerPtr.reset();
}

void Server::disconnect(){
	print(debug::Ch::Net, debug::Vb::Trivial, "%s: Disconnecting", info.name.cStr());
	connectionPtr.reset();
}

}} // net::msg
} // clover
