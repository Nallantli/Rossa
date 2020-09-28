#include "../ruota/Ruota.h"

#include <boost/asio.hpp>

RUOTA_LIB_HEADER

namespace libnet
{

	RUOTA_EXT_SYM(_service_init, args)
	{
		auto service = std::make_shared<boost::asio::io_service>();
		return Symbol(static_cast<std::shared_ptr<void>>(service));
	}

	RUOTA_EXT_SYM(_socket_init, args)
	{
		auto io_service_object = static_cast<boost::asio::io_service *>(args[2].getPointer().get());
		auto sock = std::make_shared<boost::asio::ip::tcp::socket>(*io_service_object);
		boost::system::error_code ec;
		if (sock->connect(
				boost::asio::ip::tcp::endpoint(
					boost::asio::ip::address::from_string(args[0].getString()),
					NUMBER_GET_LONG(args[1].getNumber())),
				ec))
			if (ec)
				throw std::runtime_error(ec.message());
		return Symbol(static_cast<std::shared_ptr<void>>(sock));
	}

	RUOTA_EXT_SYM(_socket_send, args)
	{
		auto sock = static_cast<boost::asio::ip::tcp::socket *>(args[0].getPointer().get());
		std::string content = args[1].getString();
		boost::asio::write(*sock, boost::asio::buffer(content));
		return Symbol();
	}

	RUOTA_EXT_SYM(_socket_read, args)
	{
		auto sock = static_cast<boost::asio::ip::tcp::socket *>(args[0].getPointer().get());
		boost::asio::streambuf sb;
		boost::system::error_code ec;
		boost::asio::read(*sock, sb, ec);
		std::string str(boost::asio::buffers_begin(sb.data()), boost::asio::buffers_begin(sb.data()) + sb.data().size());
		return Symbol(str);
	}

	RUOTA_EXT_SYM(_socket_read_until, args)
	{
		auto sock = static_cast<boost::asio::ip::tcp::socket *>(args[0].getPointer().get());
		boost::asio::streambuf sb;
		if (boost::asio::read_until(*sock, sb, args[1].getString()))
		{
			std::string str(boost::asio::buffers_begin(sb.data()), boost::asio::buffers_begin(sb.data()) + sb.data().size());
			return Symbol(str);
		}
		return Symbol();
	}

	RUOTA_EXT_SYM(_socket_close, args)
	{
		auto sock = static_cast<boost::asio::ip::tcp::socket *>(args[0].getPointer().get());
		boost::system::error_code ec;
		sock->close(ec);
		if (ec)
			return Symbol(ec.message());
		return Symbol();
	}

	RUOTA_EXT_SYM(_server_init, args)
	{
		auto io_service_object = static_cast<boost::asio::io_service *>(args[1].getPointer().get());
		auto acc = std::make_shared<boost::asio::ip::tcp::acceptor>(*io_service_object, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), NUMBER_GET_LONG(args[0].getNumber())));
		return Symbol(static_cast<std::shared_ptr<void>>(acc));
	}

	RUOTA_EXT_SYM(_server_accept, args)
	{
		auto acc = static_cast<boost::asio::ip::tcp::acceptor *>(args[0].getPointer().get());
		auto io_service_object = static_cast<boost::asio::io_service *>(args[1].getPointer().get());
		auto sock = std::make_shared<boost::asio::ip::tcp::socket>(*io_service_object);
		acc->accept(*sock);
		return Symbol(static_cast<std::shared_ptr<void>>(sock));
	}
} // namespace libnet