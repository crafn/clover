#ifndef CLOVER_NET_PROTOCOL_MSG_CONNECTION_HPP
#define CLOVER_NET_PROTOCOL_MSG_CONNECTION_HPP

#include "../net.hpp"
#include "build.hpp"
#include "debug/debugprint.hpp"
#include "msg.hpp"
#include "util/hashmap.hpp"
#include "util/queue.hpp"
#include "util/string.hpp"

#include <chrono>

namespace clover {
namespace net { namespace msg {

/// Handles connection and low-level message sending
/// @todo Client support
class Listener;
class Connection : public boost::enable_shared_from_this<Connection> {
public:
	using Ptr= boost::shared_ptr<Connection>;
	using Socket= tcp::socket;
	
	using TransferCallback= std::function<void (const Msg&)>;
	
	virtual ~Connection();
	
	void registerReceivable(const Msg::Name& name, TransferCallback cb);
	void send(const Msg& msg, TransferCallback cb = [] (const Msg&){});
	
	bool isConnected() const { return connected; }
	
	real64 getTimeFromLastContact() const;

protected:
	friend class Listener;
	
	static Ptr create(asio::io_service& s){ return Ptr(new Connection(s)); }
	Socket& getSocket(){ return socket; }
	void onConnect();
	
private:
	Connection(asio::io_service& s);

	enum class ReceivingState {
		Name,
		ValueSize,
		Value
	};
	
	bool connected;
	tcp::socket socket;
	
	using clock= std::chrono::system_clock;
	std::chrono::time_point<clock> lastContactTime;

	void startReceiving(ReceivingState s);
	Msg::Name inMsgNameBuf;
	Msg::RawRawValueSize inMsgValueSizeBuf;
	Msg::RawValue inMsgValueBuf;
	
	util::HashMap<Msg::Name, TransferCallback> onReceiveCallbacks;
	
	void startSending();
	
	struct SendMsgInfo {
		using Ptr= std::unique_ptr<SendMsgInfo>;
		
		Msg msg;
		TransferCallback callback;
		
		Msg::RawMsg msgBuf;
	};
	
	bool sending;
	util::Queue<SendMsgInfo::Ptr> sendQueue;

};

}} // net::msg
} // clover

#endif // CLOVER_NET_PROTOCOL_MSG_CONNECTION_HPP