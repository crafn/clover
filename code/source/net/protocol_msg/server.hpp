#ifndef CLOVER_NET_PROTOCOL_MSG_SERVER_HPP
#define CLOVER_NET_PROTOCOL_MSG_SERVER_HPP

#include "../exception.hpp"
#include "../net.hpp"
#include "build.hpp"
#include "connection.hpp"
#include "listener.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace net { namespace msg {

struct ServerInfo {
	util::Str8 name;
	Listener::Port port;
	real64 timeoutDuration;
};

/// Handles asynchronous connection
class Server {
public:
	template <typename MsgTraits>
	using ReceiveCallback= std::function<void (const typename MsgTraits::Value&)>;
	
	template <typename MsgTraits>
	using SendCallback= std::function<void ()>;
	
	using OnConnectCallback= std::function<void ()>;
	
	Server(const ServerInfo& info_);
	virtual ~Server(){}
	
	void setOnConnectCallback(OnConnectCallback cb){ onConnectCallback= cb; }
	
	/// Enables receiving of certain type of messages
	/// @param callback Called when message is fully received
	template <typename MsgTraits>
	void registerReceivable(ReceiveCallback<MsgTraits> callback){
		auto con_cb= [=] (const Msg& msg){
			callback(msg.getValue<typename MsgTraits::Value>());
		};
		if (connectionPtr)
			connectionPtr->registerReceivable(MsgTraits::name(), con_cb);

		recvCallbacks.pushBack(RecvCallbackInfo{MsgTraits::name(), con_cb});
	}
	
	/// Sends message
	/// @param value Value to be sent asynchronously. Serializes value right away
	/// @param callback Called when value is sent
	template <typename MsgTraits>
	void send(const typename MsgTraits::Value& value, SendCallback<MsgTraits> callback= SendCallback<MsgTraits>()){
		if (!connectionPtr || !connectionPtr->isConnected())
			throw NetException("%s: send: No connection", info.name.cStr());
		Msg msg(MsgTraits::name());
		msg.setValue(value);
		connectionPtr->send(msg, [=] (const Msg& msg){
			if (callback)
				callback();
		});
	}
	
	/// Maintains connection and messaging
	void update();
	
	bool isConnected() const { return connectionPtr ? connectionPtr->isConnected() : false; }
	
private:
	void onConnect(){ if (onConnectCallback) onConnectCallback(); }
	void startListening();
	void stopListening();
	void disconnect();
	
	ServerInfo info;

	asio::io_service ioService;
	Connection::Ptr connectionPtr;
	Listener::Ptr listenerPtr;

	OnConnectCallback onConnectCallback;
	
	/// Callbacks to be called in message handling
	struct RecvCallbackInfo {
		Msg::Name name;
		Connection::TransferCallback callback;
	};

	/// Callback storage used in case of reconnection
	util::DynArray<RecvCallbackInfo> recvCallbacks;
};

}} // net::msg
} // clover

#endif // CLOVER_NET_PROTOCOL_MSG_SERVER_HPP