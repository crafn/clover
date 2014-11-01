#include "connection.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace net { namespace msg {

Connection::Connection(asio::io_service& s)
		: connected(false)
		, socket(s)
		, sending(false){
	registerReceivable(PingMsgTraits::name(), [=] (const Msg&){
		// Answer to ping
		Msg msg(PongMsgTraits::name());
		send(msg);
	});
	
	print(debug::Ch::Net, debug::Vb::Trivial, "Connection created");
}

Connection::~Connection(){
	boost::system::error_code err;
	socket.shutdown(tcp::socket::shutdown_both, err); // Don't care about socket errors at this point
	socket.close(err);
	print(debug::Ch::Net, debug::Vb::Trivial, "Connection destroyed");
}

void Connection::registerReceivable(const Msg::Name& name, TransferCallback cb){
	ensure(onReceiveCallbacks.find(name) == onReceiveCallbacks.end());
	onReceiveCallbacks[name]= cb;
}

void Connection::send(const Msg& msg, TransferCallback cb){
	if (!connected)
		throw NetException("Not connected");
	
	sendQueue.push(std::move(SendMsgInfo::Ptr(new SendMsgInfo{msg, cb, msg.getRawMsg()})));
	
	if (!sending)
		startSending();
}

real64 Connection::getTimeFromLastContact() const {
	return (real64)std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - lastContactTime).count()/1000.0;
}

void Connection::onConnect(){
	connected= true;
	lastContactTime= clock::now();
	startReceiving(ReceivingState::Name);
}

void Connection::startReceiving(ReceivingState state){
	if (!connected) return;
	
	auto self(shared_from_this());
	
	uint8* buf= nullptr;
	SizeType bufSize= 0;
	
	if (state == ReceivingState::Name){
		buf= inMsgNameBuf.data();
		bufSize= inMsgNameBuf.size();
	}
	else if (state == ReceivingState::ValueSize){
		buf= inMsgValueSizeBuf.data();
		bufSize= inMsgValueSizeBuf.size();
	}
	else if (state == ReceivingState::Value){
		ensure(!inMsgValueBuf.empty());
		buf= &inMsgValueBuf[0];
		bufSize= inMsgValueBuf.size();
	}

	ensure(buf);

	asio::async_read(socket, asio::buffer(buf, bufSize),
		[this, self, state, buf] (	const boost::system::error_code& error,
									SizeType transferred){
			
			if (!error){
				lastContactTime= clock::now();
				
				if (state == ReceivingState::Name){
					if (transferred != Msg::nameSize)
						throw NetException("Received name has wrong size: %i != %i", transferred, Msg::nameSize);

					util::Str8 str;
					for (SizeType i= 0; i < transferred; ++i)
						str += util::Str8::format("%c", buf[i]);
						
					//print(debug::Ch::Net, debug::Vb::Trivial, "Msg name: %s", str.cStr());

					// Name is now in msgNameBuf
					
					if (onReceiveCallbacks.find(inMsgNameBuf) == onReceiveCallbacks.end())
						throw NetException("Unknown msg name: %s", inMsgNameBuf.data());
					
					startReceiving(ReceivingState::ValueSize);
				}
				else if (state == ReceivingState::ValueSize){
					if (transferred != sizeof(Msg::RawValueSize))
						throw NetException("Received value size has wrong size: %i != %i", transferred, sizeof(Msg::RawValueSize));
					
					inMsgValueBuf.resize(Msg::rawValueSize(inMsgValueSizeBuf));
					
					//print(debug::Ch::Net, debug::Vb::Trivial, "Value size: %i", inMsgValueBuf.size());
					
					if (inMsgValueBuf.empty()){ // No value
						Msg msg(inMsgNameBuf);
						onReceiveCallbacks[inMsgNameBuf](msg);
						
						startReceiving(ReceivingState::Name);
					}
					else
						startReceiving(ReceivingState::Value);
				}
				else if (state == ReceivingState::Value){
					if (transferred != inMsgValueBuf.size())
						throw NetException("Received raw value has wrong size: %i != %i", transferred, inMsgValueBuf.size());
					
					util::Str8 str;
					for (SizeType i= 0; i < transferred; ++i)
						str += util::Str8::format("%c", buf[i]);
						
					//print(debug::Ch::Net, debug::Vb::Trivial, "Value: %s", str.cStr());
					
					// Assemble the message
					Msg msg(inMsgNameBuf);
					msg.setRawValue(inMsgValueBuf);
					if (onReceiveCallbacks[inMsgNameBuf])
						onReceiveCallbacks[inMsgNameBuf](msg);
					
					startReceiving(ReceivingState::Name); // Wait for new message
				}
				else {
					throw NetException("Invalid ReceivingState");
				}

			}
			else {
				throw NetException("asio::async_read: %s", error.message().c_str());
			};
		});
}

void Connection::startSending(){
	ensure(!sendQueue.empty());
	auto self(shared_from_this());
	
	sending= true;
	asio::async_write(socket, asio::buffer(&sendQueue.front()->msgBuf[0], sendQueue.front()->msgBuf.size()),
		[this, self] (	const boost::system::error_code& error,
						SizeType transferred){

			if (!error){
				sending= false;
				
				ensure(!sendQueue.empty());
				SendMsgInfo* info= sendQueue.front().get();
				
				ensure(info != nullptr);
				ensure_msg(transferred == info->msgBuf.size(), "%zu != %zu", transferred, info->msgBuf.size());
				
				if (info->callback)
					info->callback(info->msg);
				sendQueue.pop();
				
				if (!sendQueue.empty())
					startSending();
			}
			else {
				throw NetException("asio::async_write: %s", error.message().c_str());
			}
		});
}

}} // net::msg
} // clover