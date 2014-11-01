#ifndef CLOVER_NET_PROTOCOL_MSG_LISTENER_HPP
#define CLOVER_NET_PROTOCOL_MSG_LISTENER_HPP

#include "../net.hpp"
#include "build.hpp"
#include "connection.hpp"

namespace clover {
namespace net { namespace msg {

/// Waits incoming connection
class Listener : public boost::enable_shared_from_this<Listener> {
public:
	using Ptr= boost::shared_ptr<Listener>;
	using Port= uint16;
	using Acceptor= tcp::acceptor;
	
	using OnConnectCallback= std::function<void ()>;
	
	static Ptr create(asio::io_service& s, Port port){ return Ptr(new Listener(s, port)); }

	Connection::Ptr listen(OnConnectCallback cb){
		Connection::Ptr con= Connection::create(acceptor.get_io_service());
		
		acceptor.async_accept(	con->getSocket(),
								boost::bind(&Listener::handleConnection, this, con, cb, asio::placeholders::error));

		return con;
	}
	
private:
	Listener(asio::io_service& s, Port port)
		: acceptor(s, tcp::endpoint(tcp::v4(), port)){}
		
	void handleConnection(Connection::Ptr con, OnConnectCallback cb, const boost::system::error_code& error){
		if (!error){
			con->onConnect();
			cb();
		}
		else
			throw global::Exception("Error on connecting: %s", error.message().c_str());
	}
		
	tcp::acceptor acceptor;
};

}} // net::msg
} // clover

#endif // CLOVER_NET_PROTOCOL_MSG_LISTENER_HPP