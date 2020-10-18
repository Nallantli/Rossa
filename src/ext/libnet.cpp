#include "../ruota/Ruota.h"

#include <boost/asio.hpp>

RUOTA_LIB_HEADER

namespace libnet
{
	RUOTA_EXT_SYM(_service_init, args, token, hash)
	{
		auto service = std::make_shared<boost::asio::io_service>();
		return Symbol(static_cast<std::shared_ptr<void>>(service));
	}

	RUOTA_EXT_SYM(_socket_init, args, token, hash)
	{
		auto io_service_object = COERCE_PTR(
			args[2].getPointer(token),
			boost::asio::io_service);

		auto sock = std::make_shared<boost::asio::ip::tcp::socket>(*io_service_object);
		boost::system::error_code ec;
		if (sock->connect(
			boost::asio::ip::tcp::endpoint(
				boost::asio::ip::address::from_string(args[0].getString(token)),
				args[1].getNumber(token).getLong()),
			ec))
			if (ec)
				throw RTError(ec.message(), *token);
		return Symbol(static_cast<std::shared_ptr<void>>(sock));
	}

	RUOTA_EXT_SYM(_socket_send, args, token, hash)
	{
		auto sock = COERCE_PTR(
			args[0].getPointer(token),
			boost::asio::ip::tcp::socket);

		std::string content = args[1].getString(token);
		boost::asio::write(*sock, boost::asio::buffer(content));
		return Symbol();
	}

	RUOTA_EXT_SYM(_socket_read, args, token, hash)
	{
		auto sock = COERCE_PTR(
			args[0].getPointer(token),
			boost::asio::ip::tcp::socket);

		boost::asio::streambuf sb;
		boost::system::error_code ec;
		boost::asio::read(*sock, sb, ec);
		std::string str(boost::asio::buffers_begin(sb.data()), boost::asio::buffers_begin(sb.data()) + sb.data().size());
		return Symbol(str);
	}

	RUOTA_EXT_SYM(_socket_read_until, args, token, hash)
	{
		auto sock = COERCE_PTR(
			args[0].getPointer(token),
			boost::asio::ip::tcp::socket);

		boost::asio::streambuf sb;
		if (boost::asio::read_until(*sock, sb, args[1].getString(token))) {
			std::string str(boost::asio::buffers_begin(sb.data()), boost::asio::buffers_begin(sb.data()) + sb.data().size());
			return Symbol(str);
		}
		return Symbol();
	}

	RUOTA_EXT_SYM(_socket_close, args, token, hash)
	{
		auto sock = COERCE_PTR(
			args[0].getPointer(token),
			boost::asio::ip::tcp::socket);

		boost::system::error_code ec;
		sock->close(ec);
		if (ec)
			return Symbol(ec.message());
		return Symbol();
	}

	RUOTA_EXT_SYM(_server_init, args, token, hash)
	{
		auto io_service_object = COERCE_PTR(
			args[1].getPointer(token),
			boost::asio::io_service);

		auto acc = std::make_shared<boost::asio::ip::tcp::acceptor>(*io_service_object, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), args[0].getNumber(token).getLong()));
		return Symbol(static_cast<std::shared_ptr<void>>(acc));
	}

	RUOTA_EXT_SYM(_server_accept, args, token, hash)
	{
		auto acc = COERCE_PTR(
			args[0].getPointer(token),
			boost::asio::ip::tcp::acceptor);

		auto io_service_object = COERCE_PTR(
			args[1].getPointer(token),
			boost::asio::io_service);

		auto sock = std::make_shared<boost::asio::ip::tcp::socket>(*io_service_object);
		acc->accept(*sock);
		return Symbol(static_cast<std::shared_ptr<void>>(sock));
	}
}