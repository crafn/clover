#ifndef CLOVER_NET_NET_HPP
#define CLOVER_NET_NET_HPP

/// Common header for network stuff

#include "build.hpp"
#include "global/exception.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace clover {
namespace net {

using boost::asio::ip::tcp; // Usage: net::tcp::socket
namespace asio= boost::asio; // Usage: net::asio::io_service

} // net
} // clover

#endif // CLOVER_NET_NET_HPP