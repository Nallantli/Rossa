#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include "encode.h"
#include "../main/mediator/mediator.h"

ROSSA_EXT_SIG(_service_init, args)
{
	auto service = std::make_shared<boost::asio::io_service>();
	return MAKE_POINTER(service);
}

ROSSA_EXT_SIG(_socket_init, args)
{
	auto io_service_object = COERCE_POINTER(args[2], boost::asio::io_service);

	auto sock = std::make_shared<boost::asio::ip::tcp::socket>(*io_service_object);
	boost::system::error_code ec;
	if (sock->connect(
			boost::asio::ip::tcp::endpoint(
				boost::asio::ip::address::from_string(COERCE_STRING(args[0])),
				COERCE_NUMBER(args[1]).getLong()),
			ec))
		if (ec)
			throw library_error_t(ec.message());
	return MAKE_POINTER(sock);
}

ROSSA_EXT_SIG(_socket_send, args)
{
	auto sock = COERCE_POINTER(args[0], boost::asio::ip::tcp::socket);

	std::string content = COERCE_STRING(args[1]);
	boost::asio::write(*sock, boost::asio::buffer(content));
	return mediator_t();
}

ROSSA_EXT_SIG(_socket_read, args)
{
	auto sock = COERCE_POINTER(args[0], boost::asio::ip::tcp::socket);

	boost::asio::streambuf sb;
	boost::system::error_code ec;
	boost::asio::read(*sock, sb, ec);
	std::string str(boost::asio::buffers_begin(sb.data()), boost::asio::buffers_begin(sb.data()) + sb.data().size());
	return MAKE_STRING(str);
}

ROSSA_EXT_SIG(_socket_read_until, args)
{
	auto sock = COERCE_POINTER(args[0], boost::asio::ip::tcp::socket);

	boost::asio::streambuf sb;
	if (boost::asio::read_until(*sock, sb, COERCE_STRING(args[1])))
	{
		std::string str(boost::asio::buffers_begin(sb.data()), boost::asio::buffers_begin(sb.data()) + sb.data().size());
		return MAKE_STRING(str);
	}
	return mediator_t();
}

ROSSA_EXT_SIG(_socket_close, args)
{
	auto sock = COERCE_POINTER(args[0], boost::asio::ip::tcp::socket);

	boost::system::error_code ec;
	sock->close(ec);
	if (ec)
		return MAKE_STRING(ec.message());
	return mediator_t();
}

ROSSA_EXT_SIG(_server_init, args)
{
	auto io_service_object = COERCE_POINTER(args[1], boost::asio::io_service);

	auto acc = std::make_shared<boost::asio::ip::tcp::acceptor>(*io_service_object, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), COERCE_NUMBER(args[0]).getLong()));
	return MAKE_POINTER(acc);
}

ROSSA_EXT_SIG(_server_accept, args)
{
	auto acc = COERCE_POINTER(args[0], boost::asio::ip::tcp::acceptor);
	auto io_service_object = COERCE_POINTER(args[1], boost::asio::io_service);

	auto sock = std::make_shared<boost::asio::ip::tcp::socket>(*io_service_object);
	acc->accept(*sock);
	return MAKE_POINTER(sock);
}

ROSSA_EXT_SIG(_tcp_stream_init, args)
{
	const std::string address = COERCE_STRING(args[0]);
	const std::string port = std::to_string(COERCE_NUMBER(args[1]).getLong());
	auto io_service_object = COERCE_POINTER(args[2], boost::asio::io_service);

	boost::asio::ip::tcp::resolver resolver(*io_service_object);
	auto results = resolver.resolve(address, port);

	auto tcpstream = std::make_shared<boost::beast::tcp_stream>(*io_service_object);
	boost::system::error_code ec;
	tcpstream->connect(results);

	return MAKE_POINTER(tcpstream);
}

ROSSA_EXT_SIG(_tcp_stream_request, args)
{
	const std::string target = COERCE_STRING(args[0]);
	const int version = COERCE_NUMBER(args[1]).getLong();
	const auto params = COERCE_DICTIONARY(args[2]);

	auto tcpstream = COERCE_POINTER(args[3], boost::beast::tcp_stream);

	boost::beast::http::request<boost::beast::http::string_body> req(boost::beast::http::verb::get, target, version);

	for (auto &p : params)
	{
		req.set(p.first, COERCE_STRING(p.second));
	}

	boost::beast::http::write(*tcpstream, req);
	boost::beast::flat_buffer buff;
	boost::beast::http::response<boost::beast::http::string_body> res;
	boost::beast::http::read(*tcpstream, buff, res);

	std::map<const std::string, const mediator_t> ret;

	for (auto &r : res)
	{
#if BOOST_VERSION > 108000
		ret.insert({r.name_string(), MAKE_STRING(r.value())});
#else
		ret.insert({r.name_string().to_string(), MAKE_STRING(r.value().to_string())});
#endif
	}

	ret.insert({"CONTENT", MAKE_STRING(res.body())});

	return MAKE_DICTIONARY(ret);
}

ROSSA_EXT_SIG(_tcp_stream_close, args)
{
	auto tcpstream = COERCE_POINTER(args[0], boost::beast::tcp_stream);

	tcpstream->close();

	return mediator_t();
}

ROSSA_EXT_SIG(_encodeURI, args)
{
	const std::string s = COERCE_STRING(args[0]);
	return MAKE_STRING(encodeURIComponent(s));
}

ROSSA_EXT_SIG(_decodeURI, args)
{
	const std::string s = COERCE_STRING(args[0]);
	return MAKE_STRING(decodeURIComponent(s));
}

EXPORT_FUNCTIONS(lib_net)
{
	ADD_EXT(_decodeURI);
	ADD_EXT(_encodeURI);
	ADD_EXT(_server_accept);
	ADD_EXT(_server_init);
	ADD_EXT(_service_init);
	ADD_EXT(_socket_close);
	ADD_EXT(_socket_init);
	ADD_EXT(_socket_read_until);
	ADD_EXT(_socket_read);
	ADD_EXT(_socket_send);
	ADD_EXT(_tcp_stream_close);
	ADD_EXT(_tcp_stream_init);
	ADD_EXT(_tcp_stream_request);
}